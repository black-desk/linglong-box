#include <spdlog/spdlog.h>

#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/mman.h>

#include "container.h"
#include "util/exception.h"
#include "util/sync.h"
#include "util/fd.h"

namespace linglong {

Container::Init::Init(Container *const container)
    : container(container)
{
    const auto &config = this->container->config;
}

void Container::Init::init(pid_t ppid) noexcept
{
    int ret = -1;

    // TODO: setup signal handlers

    makeSureParentSurvive(ppid);

    ret = prctl(PR_SET_CHILD_SUBREAPER, 1);
    if (ret) {
        spdlog::warn("Failed to set CHILD_SUBREAPER: {}", strerror(errno));
    }
}

void Container::Init::run()
{
    const auto &config = this->container->config;
    int ppid = getppid();

    int cloneFlag = 0;
    char *stack = nullptr;

    for (const auto &ns : config->namespaces) {
        if (!ns.path.has_value()) {
            cloneFlag |= ns.type;
        }
    }

    stack = (char *)mmap(NULL, this->container->option.StackSize, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        throw util::RuntimeError(fmt::format("Failed to allocate stack for init: {}", strerror(errno)));
    }

    stack += this->container->option.StackSize;
    char *&stackTop = stack;

    std::function<int()> lambda = [this, ppid]() -> int {
        spdlog::debug("init: start");
        this->init(ppid);

        try {
            container->rootfs->sync << 0; // request rootfs to write ID mapping

            int msg = -1;
            spdlog::debug("init: waiting rootfs to write ID mapping");
            this->sync >> msg;
            spdlog::debug("init: done");
            if (msg == -1) {
                throw util::RuntimeError("Error during waiting monitor to write ID mapping");
            }

            this->setupContainer();

            ignoreParentDie();

            this->container->rootfs->sync << 0; // NOTE: create result

            spdlog::debug("init: waiting monitor to request run hooks");
            this->sync >> msg;
            spdlog::debug("init: done");
            if (msg == -1) {
                throw util::RuntimeError("Error during waiting monitor to request run hooks");
            }

            makeSureParentSurvive();

            const auto &config = *this->container->config;

            if (config.hooks.has_value() && config.hooks->createContainer.has_value()) {
                for (const auto &hook : config.hooks->createContainer.value()) {
                    execHook(hook);
                }
            }

            this->container->sync << 0;

            this->pivotRoot();

            this->listenUnixSocket();

        } catch (const util::RuntimeError &e) {
            std::stringstream s;
            util::printException(s, e);
            spdlog::error("init: Unhanded exception during init running: {}", s);
        } catch (const std::exception &e) {
            spdlog::error("init: Unhanded exception during init running: {}", e.what());
        } catch (...) {
            spdlog::error("init: Unhanded exception during init running");
        }

        container->sync << -1;
        container->monitor->sync << -1;
        container->rootfs->sync << -1;
        return -1;
    };

    int initPID = clone(_, stackTop, cloneFlag, &lambda);
    if (initPID) { // parent
        this->pid = initPID;
        return;
    }
}

void Container::Init::setupContainer()
{
    this->setNS();
    this->setMounts();
    this->setCgroup();
    this->setDevices();
    this->setLink();
}

void Container::Init::setNS()
{
    const auto &nss = this->container->config->namespaces;
    int ret = -1;
    for (const auto &ns : nss) {
        if (ns.path.has_value()) {
            util::FD fd(open(ns.path->c_str(), O_RDONLY | O_CLOEXEC)); // FIXME: O_PATH?
            ret = setns(fd.fd, ns.type);
            if (ret == -1) {
                throw util::RuntimeError(
                    fmt::format("Failed to enter namespace {} (type={}): {}", ns.path, ns.type, strerror(errno)));
            }
        }
    }
}

void Container::Init::setMounts()
{
    const auto &mounts = this->container->config->mounts.value_or(std::vector<OCI::Config::Mount>());
    const auto &root = this->container->config->root.path;
    for (const auto &mount : mounts) {
        try {
            doMount(mount, root);
        } catch (const util::RuntimeError &e) {
            if (this->container->option.IgnoreMountFail) {
                std::stringstream s;
                util::printException(s, e);
                spdlog::error("init: {}", s);
            } else {
                throw e;
            }
        }
    }
}

void Container::Init::setCgroup()
{
    return;
    // TODO:
}

void Container::Init::setDevices()
{
}

void Container::Init::pivotRoot()
{
    util::FD oldRootFD(open("/", O_DIRECTORY | O_PATH));
    util::FD newRootFD(open(this->container->config->root.path.c_str(), O_DIRECTORY | O_RDONLY));

    int ret = -1;
    ret = fchdir(newRootFD.fd);
    if (ret != 0) {
        throw util::RuntimeError(fmt::format("Failed to chdir to new root: {}", strerror(errno)));
    }

    ret = syscall(SYS_pivot_root, ".", ".");

    ret = fchdir(oldRootFD.fd);
    if (ret != 0) {
        throw util::RuntimeError(fmt::format("Failed to chdir to new root: {}", strerror(errno)));
    }

    ret = mount(nullptr, ".", nullptr, MS_REC | MS_PRIVATE, nullptr);
    if (ret != 0) {
        throw util::RuntimeError(
            fmt::format("Failed to changing the propagation type of old root: {}", strerror(errno)));
    }

    try {
        ret = umount2(".", MNT_DETACH);
        if (ret != 0) {
            throw nullptr;
        }
        do {
            ret = umount2(".", MNT_DETACH);
            if (ret != 0 && errno == EINVAL) {
                break;
            }
            if (ret != 0) {
                throw nullptr;
            }
        } while (ret == 0);
    } catch (...) {
        throw util::RuntimeError(fmt::format("Failed to umount old rootfs: {}", strerror(errno)));
    }
}

void Container::Init::listenUnixSocket()
{
}

} // namespace linglong
