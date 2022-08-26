#include <iostream>
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

static void configIDMapping(pid_t target,
                            const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &uidMappings,
                            const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &gidMappings)
{
    // TODO:
}

static void execHook(const linglong::OCI::Config::Hooks::Hook &hook)
{
    // TODO:
}

static void makeSureParentSurvive(pid_t ppid) noexcept
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

static void ignoreParentDie()
{
}

namespace linglong {
Container::Container(const std::string &containerID, const std::filesystem::path &bundle,
                     const nlohmann::json &configJson, const std::filesystem::path &workingPath,
                     int createContainerSocket, const Option &option)
    : ID(containerID)
    , workingPath(workingPath)
    , bundlePath(bundle)
    , option(option)
    , monitor(new Monitor(this))
    , rootfs(new Rootfs(this))
    , init(new Init(this))
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
    } else {
        throw util::RuntimeError(fmt::format("container already started"));
    }
    return;
}

Container::Monitor::Monitor(Container *const container)
    : container(container)
{
    // TODO:
}

void Container::Monitor::run()
{
    int ppid = getppid();
    int monitorPID = fork();
    if (monitorPID) { // parent
        this->pid = monitorPID;
        return;
    }

    spdlog::debug("monitor: start");
    init(ppid);

    try {
        auto &rootfs = *this->container->rootfs;
        const auto &rootfsConfig = *this->container->config->annotations->rootfs;
        int msg;

        rootfs.run();

        spdlog::debug("monitor: waiting write id mapping request from rootfs");
        this->sync >> msg;
        spdlog::debug("monitor: done");
        if (msg) {
            throw util::RuntimeError("Error while waiting write id mapping request from rootfs");
        }

        configIDMapping(rootfs.pid, rootfsConfig.uidMappings, rootfsConfig.gidMappings);

        rootfs.sync << 0;

        const auto &config = *this->container->config;

        spdlog::debug("monitor: waiting run hooks request from runtime");
        this->sync >> msg;
        spdlog::debug("monitor: done");
        if (msg) {
            throw util::RuntimeError("Error during waiting run hooks request from runtime");
        }

        if (config.hooks.has_value() && config.hooks->createRuntime.has_value()) {
            for (const auto &hook : config.hooks->createRuntime.value()) {
                execHook(hook);
            }
        }

        container->init->sync << 0;
        spdlog::debug("monitor: waiting init to finish createContainer");
        this->sync >> msg;
        spdlog::debug("monitor: done");
        if (msg) {
            throw util::RuntimeError("Error during waiting createContainer hook");
        }

        container->sync << 0;

        spdlog::debug("monitor: waiting init to request run poststart");
        this->sync >> msg;
        spdlog::debug("monitor: done");
        if (msg) {
            throw util::RuntimeError("Error during waiting request run poststart");
        }

        if (config.hooks.has_value() && config.hooks->poststart.has_value()) {
            for (const auto &hook : config.hooks->poststart.value()) {
                execHook(hook);
            }
        }

        container->init->sync << 0;

        this->exec(); // NOTE: will not return

    } catch (const util::RuntimeError &e) {
        std::stringstream s;
        util::printException(s, e);
        spdlog::error("monitor: Unhanded exception during container monitor running: {}", s);
    } catch (const std::exception &e) {
        spdlog::error("monitor: Unhanded exception during container monitor running: {}", e.what());
    } catch (...) {
        spdlog::error("monitor: Unhanded exception during container monitor running");
    }

    container->sync << 1;
    container->rootfs->sync << 1;
    container->init->sync << 1;
    exit(-1);
}

void Container::Monitor::init(pid_t ppid) noexcept
{
    int ret = -1;

    // TODO: setup signal handlers

    makeSureParentSurvive(ppid);

    // TODO: setup systemd scope

    auto pid = getpid();
    ret = setpgid(pid, pid);
    if (ret) {
        spdlog::warn("Failed to setpgid: {} ({})", std::strerror(errno), errno);
    }

    ret = prctl(PR_SET_CHILD_SUBREAPER, 1);
    if (ret) {
        spdlog::warn("Failed to set CHILD_SUBREAPER: {}", strerror(errno));
    }
}

void Container::Monitor::exec()
{
    // TODO:
}

Container::Rootfs::Rootfs(Container *const c)
    : container(c)
    , cloneFlag(CLONE_NEWUSER | CLONE_NEWNS | SIGCHLD)
    , stackSize(1024 * 1024)
{
    // TODO:
}

void Container::Rootfs::run()
{
    int ppid = getppid();
    int rootfsPID = fork();
    if (rootfsPID) { // parent
        this->pid = monitorPID;
        return;
    }
}

void Container::Rootfs::allocateStack()
{
    char *stack =
        (char *)mmap(NULL, this->stackSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        throw util::RuntimeError(
            fmt::format("mmap child stack for rootfs preparer failed: {} (errno={})", std::strerror(errno), errno));
    }
    this->stackTop = stack + stackSize; /* Assume stack grows downward */
}

} // namespace linglong
