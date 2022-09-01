#include <iostream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <wait.h>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "container.h"

#include "nlohmann/json.hpp"
#include "util/exception.h"
#include "util/sync.h"

namespace linglong {

Container::Container(const std::string &containerID, const std::filesystem::path &bundle,
                     const nlohmann::json &configJson, const std::filesystem::path &workingPath, int socket,
                     const Option &option)
    : ID(containerID)
    , workingPath(workingPath)
    , bundlePath(bundle)
    , option(option)
    , monitor(new Monitor(this))
    , rootfs(new Rootfs(this))
    , init(new Init(this, socket))
{
    this->config.reset(new OCI::Config);
    configJson.get_to(*this->config.get());
    this->config->parse(bundlePath);

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
    // TODO:
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

void doMount(const linglong::OCI::Config::Mount &)
{
    // TODO:
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
