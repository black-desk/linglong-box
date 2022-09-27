#include <spdlog/spdlog.h>

#include <sys/prctl.h>

#include "container.h"
#include "oci/util.h"
#include "util/exception.h"
#include "util/fd.h"
namespace linglong {

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
    this->init(ppid);

    try {
        auto &rootfs = *this->container->rootfs;
        const auto &rootfsConfig = *this->container->config->annotations->rootfs;
        int msg;

        rootfs.run();

        spdlog::debug("monitor: waiting write id mapping request from rootfs");
        this->sync >> msg;
        spdlog::debug("monitor: done");
        if (msg) {
            throw std::runtime_error("Error while waiting write id mapping request from rootfs");
        }

        configIDMapping(rootfs.pid, rootfsConfig.uidMappings, rootfsConfig.gidMappings);

        rootfs.sync << 0;

        const auto &config = *this->container->config;

        spdlog::debug("monitor: waiting run hooks request from runtime");
        this->sync >> msg;
        spdlog::debug("monitor: done");
        if (msg) {
            throw std::runtime_error("Error during waiting run hooks request from runtime");
        }

        if (config.hooks.has_value() && config.hooks->createRuntime.has_value()) {
            for (const auto &hook : config.hooks->createRuntime.value()) {
                linglong::OCI::execHook(hook);
            }
        }

        container->init->sync << 0;
        spdlog::debug("monitor: waiting init to finish createContainer");
        this->sync >> msg;
        spdlog::debug("monitor: done");
        if (msg) {
            throw std::runtime_error("Error during waiting createContainer hook");
        }

        container->sync << 0;

        try {
            spdlog::debug("monitor: waiting init to request run poststart");
            this->sync >> msg;
            spdlog::debug("monitor: done");
            if (msg) {
                throw std::runtime_error("Error during waiting request run poststart");
            }

            if (config.hooks.has_value() && config.hooks->poststart.has_value()) {
                for (const auto &hook : config.hooks->poststart.value()) {
                    execHook(hook);
                }
            }

            ignoreParentDie();

            container->init->sync << 0;

            this->exec(); // NOTE: will not return
        } catch (const std::exception &e) {
            std::stringstream s;
            util::printException(s, e);
            spdlog::error("monitor: Unhanded exception occur after exec process: {}", s);
        } catch (...) {
            spdlog::error("monitor: Unhanded exception occur after exec process");
        }

    } catch (const std::runtime_error &e) {
        std::stringstream s;
        util::printException(s, e);
        spdlog::error("monitor: Unhanded exception during container monitor running: {}", s);
    } catch (const std::exception &e) {
        spdlog::error("monitor: Unhanded exception during container monitor running: {}", e.what());
    } catch (...) {
        spdlog::error("monitor: Unhanded exception during container monitor running");
    }

    container->sync << -1;
    container->rootfs->sync << -1;
    container->init->sync << -1;
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
    int fdlimit = (int)sysconf(_SC_OPEN_MAX);
    for (int i = STDERR_FILENO + 1; i < fdlimit; i++) {
        close(i);
    }

    auto initPidStr = std::to_string(this->container->state.pid);
    auto rootfsPidStr = std::to_string(this->container->rootfs->pid);
    auto configStr = nlohmann::json(*this->container->config).dump();

    const char *args[5] = {"monitor", initPidStr.c_str(), rootfsPidStr.c_str(), configStr.c_str()};

    int ret = execve("/proc/self/exe", const_cast<char *const *>(args), environ);
    if (ret) {
        spdlog::error("Failed to exec monitor: {}", strerror(errno));
        return;
    }
}
} // namespace linglong
