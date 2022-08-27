#include <spdlog/spdlog.h>

#include <sys/prctl.h>

#include "container.h"
#include "util/exception.h"
#include "util/sync.h"

namespace linglong {

Container::Rootfs::Rootfs(Container *const c)
    : container(c)
    , unshareFlag(CLONE_NEWUSER | CLONE_NEWNS)
{
    // TODO:
}

void Container::Rootfs::run()
{
    int ppid = getppid();
    int rootfsPID = fork();
    if (rootfsPID) { // parent
        this->pid = rootfsPID;
        return;
    }

    spdlog::debug("rootfs: start");
    this->init(ppid);

    try {
        int ret = -1;
        ret = unshare(this->unshareFlag);
        if (ret != 0) {
            throw util::RuntimeError("Failed to unshare user and mount namespace");
        }

        container->monitor->sync << 0; // request monitor to write ID mapping

        int msg = -1;
        spdlog::debug("rootfs: waiting monitor to write ID mapping");
        this->sync >> msg;
        spdlog::debug("rootfs: done");
        if (msg == -1) {
            throw util::RuntimeError("Error during waiting monitor to write ID mapping");
        }

        this->prepareRootfs();

        container->init->run();

        spdlog::debug("rootfs: waiting init to request write ID mapping");
        this->sync >> msg;
        spdlog::debug("rootfs: done");
        if (msg == -1) {
            throw util::RuntimeError("Error during waiting init to request write ID mapping");
        }

        configIDMapping(container->init->pid, container->config->uidMappings, container->config->gidMappings);

        container->init->sync << 0;

        this->exec();
        exit(-1);
    } catch (const util::RuntimeError &e) {
        std::stringstream s;
        util::printException(s, e);
        spdlog::error("rootfs: Unhanded exception during rootfs preparer running: {}", s);
    } catch (const std::exception &e) {
        spdlog::error("rootfs: Unhanded exception during rootfs preparer running: {}", e.what());
    } catch (...) {
        spdlog::error("rootfs: Unhanded exception during rootfs preparer running");
    }

    container->sync << -1;
    container->monitor->sync << -1;
    container->init->sync << -1;
    exit(-1);
}

void Container::Rootfs::init(pid_t ppid) noexcept
{
    makeSureParentSurvive(ppid);
}

void Container::Rootfs::prepareRootfs()
{
    const auto &config = this->container->config;

    if (!config->annotations.has_value()) {
        return;
    }

    if (this->container->config->annotations->rootfs.has_value()
        && this->container->config->annotations->rootfs->dbus.has_value()) {
        const auto &dbusConfigs = this->container->config->annotations->rootfs->dbus.value();

        for (const auto &dbusConfig : dbusConfigs) {
            linglong::OCI::Config::Hooks::Hook busHook = {
                dbusProxyPath(),
                std::vector<std::string>({dbusProxyPath(), "-c", dbusConfig.config.string(),
                                          dbusConfig.container.string() + ":" + dbusConfig.host.string()}),
                environPassThrough(), -1};

            try {
                execHook(busHook);
            } catch (...) {
                // TODO:
            }
        }
    }

    if (config->annotations->rootfs.has_value() && config->annotations->rootfs->overlayfs.has_value()) {
        const auto &overlayfsConfig = config->annotations->rootfs->overlayfs;

        linglong::OCI::Config::Hooks::Hook overlayfsHook = {
            fuseOverlayfsPath(),
            std::vector<std::string>({
                fuseOverlayfsPath(),
                "-o",
                fmt::format("lowerdir={}",
                            fmt::join(overlayfsConfig->lower.begin(), overlayfsConfig->lower.end(), ":")),
                "-o",
                fmt::format("upperdir={}", overlayfsConfig->upper),
                "-o",
                fmt::format("workdir={}", overlayfsConfig->workdir),
                config->root.path,
            }),
            environPassThrough(), -1};

        try {
            execHook(overlayfsHook);
        } catch (...) {
            // TODO:
        }
    }

    if (config->annotations->rootfs.has_value() && config->annotations->rootfs->native.has_value()) {
        for (const auto &mount : config->annotations->rootfs->native->mounts) {
            doMount(mount);
        }
    }
}

void Container::Rootfs::exec()
{
    // TODO:
}
} // namespace linglong
