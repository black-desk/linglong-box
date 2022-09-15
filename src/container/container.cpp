#include <fstream>

#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <wait.h>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "container.h"

#include "nlohmann/json.hpp"
#include "util/exception.h"
#include "util/filesystem.h"
#include "util/wait.h"

namespace linglong {

Container::Container(const std::string &containerID, const std::filesystem::path &bundle,
                     const nlohmann::json &configJson, const std::filesystem::path &workingPath, int socket,
                     const Option &option)
    : ID(containerID)
    , bundlePath(bundle)
    , option(option)
    , monitor(new Monitor(this))
    , rootfs(new Rootfs(this))
    , init(new Init(this, socket))
{
    this->config.reset(new OCI::Config);
    configJson.get_to(*this->config.get());
    this->config->parse(bundlePath);

    this->containerRoot.reset(new util::FD(open(this->config->root.path.c_str(), O_PATH | O_CLOEXEC)));

    this->state = {this->config->ociVersion, containerID, "creating", 0, bundle, OCI::Runtime::State::Annotations({0})};
};

void Container::Create()
{
    if (!this->monitor->pid) {
        this->monitor->run();

        int msg = 0;
        spdlog::debug("runtime: wait init to send create result");
        this->sync >> msg;
        spdlog::debug("runtime: done");
        if (msg == -1) {
            throw std::runtime_error("Failed to create container");
        } else {
            this->state.annotations->monitorPid = this->monitor->pid;
            this->state.pid = msg;
            this->state.status = "created";
        }
    } else {
        throw std::runtime_error(fmt::format("container already started"));
    }
    return;
}

void configIDMapping(pid_t target, const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &uidMappings,
                     const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &gidMappings)
{
    try {
        if (uidMappings.has_value()) {
            std::ofstream uidMapFile(std::filesystem::path("/proc") / std::to_string(target) / "uid_map");
            for (auto const &idMap : uidMappings.value()) {
                uidMapFile << fmt::format("{} {} {}\n", idMap.containerID, idMap.hostID, idMap.size);
            }
        }

        if (gidMappings.has_value()) {
            {
                std::ofstream setgroupsFile(std::filesystem::path("/proc") / std::to_string(target) / "setgroups");
                setgroupsFile << "deny";
            }
            std::ofstream gidMapFile(std::filesystem::path("/proc") / std::to_string(target) / "gid_map");
            for (auto const &idMap : gidMappings.value()) {
                gidMapFile << fmt::format("{} {} {}\n", idMap.containerID, idMap.hostID, idMap.size);
            }
        }

        // TODO: fallback to use newuidmap / newgidmap
    } catch (...) {
        std::throw_with_nested(std::runtime_error("config id mapping failed"));
    }
}

void makeSureParentSurvive(pid_t ppid) noexcept
{
    int ret = -1;
    ret = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (ret) {
        spdlog::warn("Failed to set PDEATHSIG: {}", strerror(errno));
    }
    if (getppid() != ppid) {
        spdlog::error("Parent has died, exit now");
        exit(-1);
    }
}

void ignoreParentDie() noexcept
{
    int ret = -1;
    ret = prctl(PR_SET_PDEATHSIG, 0);
    if (ret) {
        spdlog::warn("Failed to clear PDEATHSIG: {}", strerror(errno));
    }
}

int _(void *arg)
{
    auto fn = (std::function<int()> *)arg;
    return (*fn)();
}

std::string dbusProxyPath()
{
    // TODO:
    return "/usr/bin/ll-dbus-proxy";
}

std::string fuseOverlayfsPath()
{
    // TODO:
    return "/usr/bin/fuse-overlayfs";
}

std::vector<std::string> environPassThrough()
{
    // TODO:
    return {};
}

void doFallbackMount(const linglong::OCI::Config::Mount &m, const util::FD &root, const std::filesystem::path &rootpath,
                     const doMountOption &opt)
{
    try {
        if (m.type == OCI::Config::Mount::Type::Sysfs) {
            OCI::Config::Mount fallbackMount(nlohmann::json({
                {"source", "/sys"},
                {"destination", "/sys"},
                {"type", "bind"},
                {"option", "rbind,ro"},
            })); // should not contain any relative path
            fallbackMount.parse("/");
            doMount(fallbackMount, root, rootpath,
                    {
                        opt.ignoreError,
                        opt.resolveRealPath,
                        false,
                    });
        } else if (m.type == OCI::Config::Mount::Type::Mqueue) {
            OCI::Config::Mount fallbackMount(nlohmann::json({
                {"source", "/dev/mqueue"},
                {"destination", "/dev/mqueue"},
                {"type", "bind"},
                {"option", "rbind"},
            })); // should not contain any relative path
            fallbackMount.parse("/");
            doMount(fallbackMount, root, rootpath,
                    {
                        opt.ignoreError,
                        opt.resolveRealPath,
                        false,
                    });
        } else {
            spdlog::warn("No fallback mount confilgure found");
            throw;
        }
    } catch (...) {
        spdlog::warn("Fallback mount failed");
        throw;
    }
}

void doMount(const linglong::OCI::Config::Mount &m, const util::FD &root, const std::filesystem::path &rootpath,
             const doMountOption &opt)
{
    // https://github.com/opencontainers/runc/blob/0ca91f44f1664da834bc61115a849b56d22f595f/libcontainer/utils/utils.go#L112
    try {
        if (m.source.has_value()) {
            if (!std::filesystem::is_directory(m.source.value())) {
                util::fs::touch(rootpath / m.destination, 0644);
            } else {
                std::filesystem::create_directories(rootpath / m.destination);
            }
        }

        std::unique_ptr<util::FD> destination(new util::FD(openat(root.fd, m.destination.c_str(), O_PATH | O_CLOEXEC)));
        auto realDestination = std::filesystem::read_symlink(fmt::format("/proc/self/fd/{}", destination->fd));
        if (realDestination.string().rfind(rootpath.string(), 0) != 0) {
            throw std::runtime_error(fmt::format("possibly malicious path detected ({} vs {}), refusing to operate",
                                                 m.destination, realDestination));
        }

        std::unique_ptr<util::FD> source;
        if (m.source.has_value()) {
            std::unique_ptr<char[]> buffer = nullptr;
            const char *real = nullptr;
            if (opt.resolveRealPath) {
                buffer.reset(new char[PATH_MAX]);
                if (!buffer) {
                    spdlog::warn("Failed to malloc memory for realpath: {}", strerror(errno));
                } else {
                    real = realpath(m.source->c_str(), buffer.get());
                    if (real == nullptr) {
                        spdlog::warn("Failed to get realpath of mount source \"{}\": {}", m.source.value(),
                                     strerror(errno));
                    }
                }
            }
            source.reset(new util::FD(open(real ? real : m.source->c_str(), O_PATH | O_CLOEXEC)));
        }

        std::string sourcePath = source ? fmt::format("/proc/self/fd/{}", source->fd) : "";
        std::string destinationPath = fmt::format("/proc/self/fd/{}", destination->fd);

        auto join = [](std::string &first, const std::string &second) -> std::string & { return first += second; };
        std::string data = std::accumulate(m.parsed->data.begin(), m.parsed->data.end(), std::string(), join);

        auto type = m.type.has_value() ? to_string(nlohmann::json(m.type.value())) : "";

        try {
            auto ret = mount(sourcePath.c_str(), realDestination.c_str(), type.c_str(), m.parsed->flags, data.c_str());
            if (ret) {
                throw std::runtime_error(fmt::format(
                    "syscall mount (source=\"{}\",destination=\"{}\",filesystem=\"{}\",flags={},data={}) failed: {}",
                    sourcePath, realDestination, nlohmann::json(m.type), m.parsed->flags, data.c_str(),
                    strerror(errno)));
            }
        } catch (std::runtime_error &e) {
            if (opt.fallback) {
                std::stringstream buffer;
                util::printException(buffer, e);
                spdlog::warn("mount [{}] failed: {}, try fallback now", m, buffer);
                doFallbackMount(m, root, rootpath, opt);
            } else {
                throw;
            }
        }

        bool needRemount = (data.empty() && (m.parsed->flags & ~(MS_BIND | MS_REC | MS_REMOUNT)) == 0);
        if (needRemount) {
            auto ret = mount(sourcePath.c_str(), realDestination.c_str(), type.c_str(), m.parsed->flags | MS_REMOUNT,
                             data.c_str());
            if (ret) {
                throw std::runtime_error(fmt::format(
                    "syscall mount (source=\"{}\",destination=\"{}\",filesystem=\"{}\",flags={},data={}) failed: {}",
                    sourcePath, realDestination, nlohmann::json(m.type), m.parsed->flags | MS_REMOUNT, data.c_str(),
                    strerror(errno)));
            }
        }
    } catch (std::runtime_error &e) {
        if (opt.ignoreError) {
            std::stringstream buffer;
            util::printException(buffer, e);
            spdlog::error("Failed to preform mount [{}]: {}", m, buffer);
        } else {
            std::throw_with_nested(fmt::format("Failed to preform mount [{}]", m));
        }
    }
}

void doUmount(const linglong::OCI::Config::Mount &m, const util::FD &root)
{
    std::unique_ptr<util::FD> destination(new util::FD(openat(root.fd, m.destination.c_str(), O_PATH | O_CLOEXEC)));
    auto destinationFDPath = fmt::format("/proc/self/fd/{}", destination->fd);
    auto ret = umount(destinationFDPath.c_str());
    if (ret) {
        throw std::runtime_error(fmt::format("Failed to umount [{}]: {}", m, strerror(errno)));
    }
}

ContainerRef::ContainerRef(const std::filesystem::path &workingPath)
    : workingPath(workingPath)
    , socket(this->connect())
{
    auto stateJsonPath = workingPath / std::filesystem::path("state.json");
    std::ifstream stateJsonFile;
    stateJsonFile.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    stateJsonFile.open(stateJsonPath);

    nlohmann::json stateJson;
    stateJsonFile >> stateJson;
    stateJson.get_to(this->state);

    if (this->state.pid != 0) {
        if (kill(this->state.pid, 0)) {
            if (errno != ESRCH) {
                throw fmt::system_error(errno, "failed to call kill({}, 0)", this->state.pid);
            } else {
                this->state.pid = 0;
                this->state.status = "stopped";
            }
        }
    }
}

void ContainerRef::Start()
{
    util::Message msg({{"command", "start"}, {}});
    socket << msg;
    socket >> msg;

    if (msg.raw.get<int>()) {
        throw std::runtime_error("start container failed");
    }

    this->state.status = "running";
    this->terminalFD = msg.fds.empty() ? nullptr : msg.fds.front();
}

void ContainerRef::Kill(const int &sig)
{
    if (this->state.status != "created" && this->state.status != "running") {
        throw fmt::system_error(EINVAL, "Cannot kill container neither \"created\" nor \"running\"");
    }
    if (kill(this->state.pid, sig)) {
        throw fmt::system_error(errno, "failed to call kill({}, {})", this->state.pid, sig);
    }
}

void ContainerRef::Delete()
{
    int &monitor = this->state.annotations->monitorPid;
    if (this->state.status != "stopped") {
        throw fmt::system_error(EINVAL, "Cannot delete container not \"stopped\"");
    }
    if (kill(-monitor, SIGTERM)) {
        throw fmt::system_error(errno, "failed to call kill({}, {})", -monitor, SIGTERM);
    }
}

void ContainerRef::Exec()
{
    
}
} // namespace linglong
