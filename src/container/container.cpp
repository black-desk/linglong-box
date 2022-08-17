

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
#include "util/socket.h"

namespace linglong {

static int doCreate(void *arg);

Container::Container(const std::string &containerID, const std::filesystem::path &bundle,
                     const nlohmann::json &configJson, const std::filesystem::path &workingPath, const Option &option)
    : ID(containerID)
    , workingPath(workingPath)
    , bundlePath(bundle)
    , option(option)
    , isRef(false)
{
    this->config.reset(new OCI::Config);
    configJson.get_to(*this->config.get());
    this->config->parse(bundlePath);
}

void Container::Create()
{
    if (this->isRef) {
        throw util::RuntimeError(fmt::format("Cannot call create on a container reference."));
    }

    auto createSockets = util::SocketPair();
    auto hooksSockets = util::SocketPair();

    this->monitor.reset(
        new Monitor(this->workingPath, std::move(this->config), createSockets.second, hooksSockets.second));
    int monitorPID = fork();
    if (monitorPID) { // parent
        int ret;

        ret = close(createSockets.second);
        if (ret) {
            spdlog::error("Failed to close created message socket: {} ({})", strerror(errno), errno);
        }

        ret = close(hooksSockets.second);
        if (ret) {
            spdlog::error("Failed to close create hooks message socket: {} ({})", strerror(errno), errno);
        }

        util::Socket(createSockets.first) >> ret;

        if (ret != 0) {
            throw util::RuntimeError(fmt::format("Failed to create container (ID=\"{}\")", this->ID));
        }

        this->hooksSocket = hooksSockets.first;

        return;

    } else { // child
        this->monitor->run(); // NOTE: should not return
        exit(-1);
    }
}

void Container::waitCreateHooks()
{
    int ret = -1;
    util::Socket(hooksSocket) >> ret;
    if (ret != 0) {
        throw util::RuntimeError(fmt::format("Failed to run create hooks for container (ID=\"{}\")", this->ID));
    }
}

Container::Monitor::Monitor(const std::filesystem::path &workingPath, std::unique_ptr<OCI::Config> config,
                            int createSocket, int hookSocket)
    : workingPath(workingPath)
    , pid1(new PID1(*config, this->workingPath / "socket"))
    , rootfs(new Rootfs(config->annotations.has_value()
                            ? config->annotations->rootfs.value_or(OCI::Config::Annotations::Rootfs())
                            : OCI::Config::Annotations::Rootfs()))
{
    auto initPIDSockets = util::SocketPair();
    auto createContainerSockets = util::SocketPair();
    auto poststartSockets = util::SocketPair();

    this->pid1->createContainerSocket = createContainerSockets.second;
    this->pid1->poststartSocket = poststartSockets.second;
    this->pid1->writeIDMappingSocket = initPIDSockets.second;

    this->epoll.registerFD(initPIDSockets.first, EPOLLIN,
                           std::bind(&Container::Monitor::handleInitPID, this, std::placeholders::_1));
    this->epoll.registerFD(createContainerSockets.first, EPOLLIN,
                           std::bind(&Container::Monitor::handleCreateContainer, this, std::placeholders::_1));
    this->epoll.registerFD(poststartSockets.first, EPOLLIN,
                           std::bind(&Container::Monitor::handlePoststart, this, std::placeholders::_1));
}

Container::PID1::~PID1()
{
    int ret = -1;

    ret = close(this->createContainerSocket);
    if (ret) {
        spdlog::error("Failed to close createdContainer socket: {} ({})", strerror(errno), errno);
    }

    ret = close(this->poststartSocket);
    if (ret) {
        spdlog::error("Failed to close poststart socket: {} ({})", strerror(errno), errno);
    }

    ret = close(this->writeIDMappingSocket);
    if (ret) {
        spdlog::error("Failed to close writeIDMapping socket: {} ({})", strerror(errno), errno);
    }
}

void Container::Monitor::run()
{
    try {
        int ret = -1;

        // TODO: maybe setup systemd scope?

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

        this->rootfs->run();

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

void Container::Monitor::initSignalHandler()
{
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

void Container::Rootfs::setup(PID1 &pid1)
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
