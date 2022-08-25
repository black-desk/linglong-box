

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <wait.h>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "container.h"

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
}

void Container::Create()
{
    if (!this->monitor->pid) {
        this->monitor->run();
    } else {
        throw util::RuntimeError(fmt::format("container already started"));
    }
    return;
}

Container::Monitor::Monitor(const Container *const container)
    : container(container)
{
    // TODO
}

void Container::Monitor::run()
{
    int monitorPID = fork();
    if (monitorPID) { // parent
        this->pid = monitorPID;
        return;
    }

    try {
        spdlog::debug("monitor: start");
        init();

        auto &rootfs = *this->container->rootfs;
        const auto &rootfsConfig = *this->container->config->annotations->rootfs;
        int msg;

        rootfs.run();

        spdlog::debug("monitor: waiting write id mapping request from rootfs");
        rootfs.sync >> msg;
        spdlog::debug("monitor: done");

        configIDMapping(rootfs.pid, rootfsConfig.uidMappings, rootfsConfig.gidMappings);



        const auto &config = *this->container->config;

        spdlog::debug("monitor: waiting run hooks request from runtime");
        sync >> msg;
        spdlog::debug("monitor: done");

        if (config.hooks->createRuntime.has_value()) {
            for (const auto &hook : config.hooks->createRuntime.value()) {
                execHook(hook);
            }
        }

        container->init->sync << 1;
        spdlog::debug("monitor: wait init to finish createContainer");
        container->init->sync >> msg;
        spdlog::debug("monitor: done");

    } catch (const util::RuntimeError &e) {
        std::stringstream s;
        util::printException(s, e);
        spdlog::error("Unhanded exception during container monitor running: {}", s);
    } catch (const std::exception &e) {
        spdlog::error("Unhanded exception during container monitor running: {}", e.what());
    } catch (...) {
        spdlog::error("Unhanded exception during container monitor running");
    }
    exit(-1);
}

void Container::Monitor::init()
{
    int ret = -1;

    // TODO: setup systemd scope

    auto pid = getpid();
    ret = setpgid(pid, pid);
    if (ret) {
        spdlog::warn("Failed to setpgid: {} ({})", std::strerror(errno), errno);
    }

    ret = prctl(PR_SET_CHILD_SUBREAPER, 1);
    if (ret) {
        spdlog::warn("Failed to set CHILD_SUBREAPER");
    }

    this->initSignalHandler();

    // TODO:
    // 1. block signal will be listen by signalfd later like SIGCHLD.
    // 2. register signal handler for other signal.
}

Container::Rootfs::Rootfs(const OCI::Config::Annotations::Rootfs &config)
    : cloneFlag(CLONE_NEWUSER | CLONE_NEWNS | SIGCHLD)
    , stackSize(1024 * 1024)
{
}

static int _(void *arg)
{
}

void Container::Rootfs::setup(Init &pid1)
{
    this->allocateStack();
    this->createNamespaces();
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
