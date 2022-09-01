#define _XOPEN_SOURCE 500
#define _GNU_SOURCE

#include <optional>
#include <cstdlib>

#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <sys/sysmacros.h>
#include <sys/socket.h>

#include <spdlog/spdlog.h>

#include "container.h"
#include "util/exception.h"
#include "util/sync.h"
#include "util/fd.h"

static const uint PTSNAME_LEN = 64;

namespace linglong {

Container::Init::Init(Container *const container, int socket)
    : container(container)
    , socket(socket)
{
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
        throw std::runtime_error(fmt::format("Failed to allocate stack for init: {}", strerror(errno)));
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
                throw std::runtime_error("Error during waiting monitor to write ID mapping");
            }

            this->setupContainer();

            ignoreParentDie();

            this->container->rootfs->sync << 0; // NOTE: create result

            spdlog::debug("init: waiting monitor to request run hooks");
            this->sync >> msg;
            spdlog::debug("init: done");
            if (msg == -1) {
                throw std::runtime_error("Error during waiting monitor to request run hooks");
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

            this->waitStart();
        } catch (const std::runtime_error &e) {
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
    this->setLink();
    this->setCgroup();
    this->setSeccomp();
    this->setDevices();
    this->setConsole();
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
                throw std::runtime_error(
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
        doMount(mount, root, this->container->option.IgnoreMountFail);
    }
}

void Container::Init::setCgroup()
{
    spdlog::error("init: TODO: cgroup support not implemented yet");
    // TODO:
    return;
}

void Container::Init::setSeccomp()
{
    spdlog::error("init: TODO: seccomp support not implemented yet");
    // TODO:
    return;
}

void Container::Init::setDevices()
{
    struct std::vector<OCI::Config::Device> neededDevs({
        {OCI::Config::Device::Char, "/dev/null", 1, 3, 0666, 0, 0},
        {OCI::Config::Device::Char, "/dev/zero", 1, 5, 0666, 0, 0},
        {OCI::Config::Device::Char, "/dev/full", 1, 7, 0666, 0, 0},
        {OCI::Config::Device::Char, "/dev/tty", 5, 0, 0666, 0, 0},
        {OCI::Config::Device::Char, "/dev/random", 1, 8, 0666, 0, 0},
        {OCI::Config::Device::Char, "/dev/urandom", 1, 9, 0666, 0, 0},
    });

    for (const auto &d : neededDevs) {
        if (this->container->option.CreateDefaultDevice) {
            dev_t dev = -1;
            dev = makedev(d.major, d.minor);
            if (dev == -1) {
                throw std::runtime_error(
                    fmt::format("Failed to makedev (major={},minor={}): {}", d.major, d.minor, strerror(errno)));
            }
            int ret = -1;
            ret =
                mknod((this->container->config->root.path / d.path).c_str(), d.fileMode.value_or(0700) | S_IFCHR, dev);
            if (ret == -1) {
                throw std::runtime_error(
                    fmt::format("Failed to makedev (major={},minor={}): {}", d.major, d.minor, strerror(errno)));
            }
        } else {
            doMount({this->container->config->root.path / d.path, d.path, std::nullopt, OCI::Config::Mount::Bind,
                     std::nullopt, std::nullopt},
                    this->container->config->root.path, this->container->option.IgnoreMountFail);
        }
    }

    // FIXME: use symbol link?
    // NOTE: ignore fail here as maybe /dev/pts might not mount
    doMount({this->container->config->root.path / "dev/ptmx", this->container->config->root.path / "dev/pts/ptmx",
             std::nullopt, OCI::Config::Mount::Bind, std::nullopt, std::nullopt},
            this->container->config->root.path, true);
}

void Container::Init::setLink()
{
    static std::vector<std::pair<const char *const, const char *const>> standardSymlinks = {
        {"/proc/self/fd", "/dev/fd"},
        {"/proc/self/fd/0", "/dev/stdin"},
        {"/proc/self/fd/1", "/dev/stdout"},
        {"/proc/self/fd/2", "/dev/stderr"},
    };

    static std::vector<std::pair<const char *const, const char *const>> lfsSymlinks = {
        {"/usr/bin", "/bin"},     {"/usr/lib", "/lib"},       {"/usr/lib32", "/lib32"},
        {"/usr/lib64", "/lib64"}, {"/usr/libx32", "/libx32"},
    };

    std::vector<std::vector<std::pair<const char *const, const char *const>> *> linksArr = {&standardSymlinks};
    if (this->container->option.LinkLFS) {
        linksArr.push_back(&lfsSymlinks);
    }

    for (auto links : linksArr) {
        for (const auto &link : *links) {
            int ret = symlink(link.first, link.second);
            if (ret == -1) {
                throw std::runtime_error(fmt::format("Failed to create symlink (from=\"{}\", to=\"{}\"): {}",
                                                     link.first, link.second, strerror(errno)));
            }
        }
    }
}

void Container::Init::setConsole()
{
    if (!(this->container->config->process.has_value() && this->container->config->process->terminal.value_or(false)))
        return;

    std::unique_ptr<util::FD> pty(new util::FD(open("/dev/ptmx", O_RDWR | O_NOCTTY | O_CLOEXEC)));

    int ret = -1;
    char buf[PTSNAME_LEN];
    ret = ptsname_r(pty->fd, buf, sizeof(buf));
    if (ret < 0)
        throw std::runtime_error(fmt::format("Failed to get ptsname: {}", strerror(errno)));

    ret = unlockpt(pty->fd);
    if (ret < 0)
        throw std::runtime_error(fmt::format("Failed to unlockpt: {}", strerror(errno)));

    if (this->container->config->process->consoleSize.has_value()) {
        const auto &size = this->container->config->process->consoleSize.value();
        struct winsize ws({size.height, size.width});

        int ret;

        ret = ioctl(pty->fd, TIOCSWINSZ, &ws);
        if (ret < 0)
            throw std::runtime_error(fmt::format("Failed to set console size (height={}, width={}): {}", size.height,
                                                 size.width, strerror(errno)));
    }

    this->terminalFD = std::move(pty);

    return;
}

void Container::Init::pivotRoot()
{
    util::FD oldRootFD(open("/", O_DIRECTORY | O_PATH));
    util::FD newRootFD(open(this->container->config->root.path.c_str(), O_DIRECTORY | O_RDONLY));

    int ret = -1;
    ret = fchdir(newRootFD.fd);
    if (ret != 0) {
        throw std::runtime_error(fmt::format("Failed to chdir to new root: {}", strerror(errno)));
    }

    ret = syscall(SYS_pivot_root, ".", ".");

    ret = fchdir(oldRootFD.fd);
    if (ret != 0) {
        throw std::runtime_error(fmt::format("Failed to chdir to new root: {}", strerror(errno)));
    }

    ret = mount(nullptr, ".", nullptr, MS_REC | MS_PRIVATE, nullptr);
    if (ret != 0) {
        throw std::runtime_error(
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
        throw std::runtime_error(fmt::format("Failed to umount old rootfs: {}", strerror(errno)));
    }
}

void Container::Init::waitStart()
{
    int ret = listen(this->socket, 1);
    if (ret) {
        throw std::runtime_error(fmt::format("Failed to listen socket: {}", strerror(errno)));
    }

    spdlog::debug("init: waiting start command");
    int startRequestFD = accept(this->socket, nullptr, nullptr);
    spdlog::debug("init: done");

    util::Pipe conn((util::FD(startRequestFD)));

    util::Message startRequest;
    conn >> startRequest;

    const auto &config = *this->container->config;

    if (config.hooks.has_value() && config.hooks->startContainer.has_value()) {
        for (const auto &hook : config.hooks->startContainer.value()) {
            execHook(hook);
        }
    }

    this->execProcess();

    try {
        this->container->monitor->sync << 0;

        int msg = -1;

        this->sync >> msg;

        util::Message startResponse({
            msg,
            {this->terminalFD->fd},
        });

        conn << startResponse;

        this->clear();

    } catch (const std::exception &e) {
        std::stringstream s;
        util::printException(s, e);
        spdlog::error("init: Unhanded exception occur after exec process: {}", s);
    } catch (...) {
        spdlog::error("init: Unhanded exception occur after exec process");
    }
}

void Container::Init::execProcess()
{
}

} // namespace linglong
