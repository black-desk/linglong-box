#include <iostream>
#include <fstream>
#include <ext/stdio_filebuf.h>

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
#include "util/sync.h"
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

    this->state = {this->config->ociVersion,
                   containerID,
                   "creating",
                   0,
                   bundle,
                   this->config->annotations.value_or(OCI::Config::Annotations()).raw};
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
    // TODO:
}

void execHook(const linglong::OCI::Config::Hooks::Hook &hook)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);

    try {
        int hookPipe[2];
        int ret = pipe(hookPipe);
        if (ret) {
            throw std::runtime_error(fmt::format("Failed to create pipe: {}", strerror(errno)));
        }

        // for clean up
        auto fd1 = util::FD(hookPipe[0]);
        auto fd2 = util::FD(hookPipe[1]);

        bool has_timeout = hook.timeout.has_value() && hook.timeout;
        time_t start = time(NULL);

        if (has_timeout) {
            ret = sigprocmask(SIG_BLOCK, &mask, NULL);
            if (ret) {
                throw std::runtime_error(fmt::format("Failed to block sigchld"));
            }
        }

        int hookPID = fork();
        if (hookPID) {
            close(hookPipe[0]);
            if (has_timeout) {
                auto timeout = hook.timeout.value();
                for (time_t now = start; now - start < timeout; now = time(nullptr)) {
                    siginfo_t info;
                    int elapsed = now - start;
                    struct timespec ts_timeout = {.tv_sec = timeout - elapsed, .tv_nsec = 0};

                    ret = sigtimedwait(&mask, &info, &ts_timeout);
                    if (ret < 0 && errno != EAGAIN)
                        throw std::runtime_error(fmt::format("Failed to call sigtimedwait: {}", strerror(errno)));

                    if (info.si_signo == SIGCHLD && info.si_pid == hookPID)
                        break;

                    if (ret < 0 && errno == EAGAIN) {
                        ret = kill(hookPID, SIGKILL);
                        if (ret) {
                            throw std::runtime_error(fmt::format("Failed to kill timeout hook: {}", strerror(errno)));
                        }
                        throw std::runtime_error(fmt::format("hook \"{}\" timeout", hook.path));
                    }
                }
            }

            int wstatus;
            ret = waitpid(hookPID, &wstatus, 0);
            if (ret) {
                throw std::runtime_error(fmt::format("Failed to call waitpid: {}", strerror(errno)));
            }

            auto hookOutput = std::string(
                std::istreambuf_iterator<char>(new __gnu_cxx::stdio_filebuf<char>(hookPipe[1], std::ios::in)),
                std::istreambuf_iterator<char>());

            auto [termed, code] = util::parse_wstatus(wstatus);

            if (!termed && code != 0) {
                throw std::runtime_error(
                    fmt::format("hook failed, exit with {}, stdout & stderr = \"{}\"", code, hookOutput));
            }
        } else {
            ret = dup2(hookPipe[0], STDOUT_FILENO);
            if (ret) {
                throw std::runtime_error(fmt::format("Failed to dup pipe to STDOUT of hook: {}", strerror(errno)));
            }
            ret = dup2(hookPipe[0], STDERR_FILENO);
            if (ret) {
                throw std::runtime_error(fmt::format("Failed to dup pipe to STDERR of hook: {}", strerror(errno)));
            }

            int fdlimit = (int)sysconf(_SC_OPEN_MAX);
            for (int i = STDERR_FILENO + 1; i < fdlimit; i++)
                close(i);

            const auto &hookArgs = hook.args.value_or(std::vector<std::string>());
            const auto &hookEnv = hook.env.value_or(std::vector<std::string>());
            const char *args[hookArgs.size() + 1];
            const char *env[hookEnv.size() + 1];

            for (int i = 0; i < hookArgs.size(); i++) {
                args[i] = hookArgs[i].c_str();
            }
            args[hookArgs.size()] = nullptr;

            for (int i = 0; i < hookEnv.size(); i++) {
                env[i] = hookEnv[i].c_str();
            }
            env[hookEnv.size()] = nullptr;

            ret = execve(hook.path.c_str(), const_cast<char *const *>(args), const_cast<char *const *>(env));
            if (ret) {
                std::cerr << "execve \"" << hook.path << "\" failed with errno==" << errno << ":" << strerror(errno)
                          << std::endl;
                exit(-1);
            }
        }

    } catch (...) {
        int ret = -1;
        ret = sigprocmask(SIG_UNBLOCK, &mask, NULL);
        if (ret) {
            std::throw_with_nested(fmt::format("Failed to unblock SIGCHLD: {}", strerror(errno)));
        }
        throw;
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

void doMount(const linglong::OCI::Config::Mount &m, const util::FD &root, const std::filesystem::path rootpath,
             bool ignoreError)
{
    // https://github.com/opencontainers/runc/blob/0ca91f44f1664da834bc61115a849b56d22f595f/libcontainer/utils/utils.go#L112
    try {
        int fd = openat(root.fd, m.destination.c_str(), O_PATH | O_CLOEXEC);
        if (fd < 0) {
            throw std::runtime_error(
                fmt::format("Failed to open \"{}\" in container: {}", m.destination, strerror(errno)));
        }

        auto realpath = std::filesystem::read_symlink(fmt::format("/proc/self/fd/{}", fd));

        if (realpath.string().rfind(rootpath.string(), 0) != 0) {
            throw std::runtime_error(fmt::format("possibly malicious path detected ({} vs {}), refusing to operate",
                                                 m.destination, realpath));
        }
        std::string data = std::accumulate(m.parsed->data.begin(), m.parsed->data.end(), ",");
        auto ret =
            mount(m.source.value().c_str(), realpath.c_str(), m.type.value().c_str(), m.parsed->flags, data.c_str());

        close(fd);

        errno = olderrno;
        return ret;
    } catch (...) {
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
}

void ContainerRef::Start()
{
    util::Message msg({{"command", "start"}, {}});
    socket << msg;

    socket >> msg;
    if (msg.raw.get<int>()) {
        throw std::runtime_error("start container failed");
    } else {
        this->state.status = "running";
        this->terminalFD = msg.fds.empty() ? -1 : std::move(msg.fds.front());
    }
}
} // namespace linglong
