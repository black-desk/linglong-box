#include <sys/prctl.h>
#include <sys/signal.h>

#include "monitor.h"
#include "util/fmt.h"
#include "util/exec.h"
#include "lib/oci/util.h"

namespace linglong::box::container {

Monitor::Monitor(OCI::Config config, util::WriteableFD runtime,
                 util::WriteableFD rootfs, util::WriteableFD init,
                 util::ReadableFD monitor)
    : sync(std::move(monitor))
    , config(config)
{
    SPDLOG_TRACE("linglong::box::container::Monitor::Monitor called");
    SPDLOG_TRACE("oci_config:\n{}", nlohmann::json(config).dump(4));

    {
        SPDLOG_TRACE("start adopt orphaned processes");
        auto ret = prctl(PR_SET_CHILD_SUBREAPER, 1);
        if (ret) {
            auto err =
                fmt::system_error(errno, "failed to set child subreaper to 1");
            SPDLOG_WARN(err.what());
            SPDLOG_WARN("MIGHT cannot release rootfs when container deleted");
        }
    }

    // NOTE:
    // ll-box-monitor just a subreaper for rootfs-preparer processes
    // and oci-runtime hook, we don't care about how child processes die.
    SPDLOG_TRACE("set SIGCHLD handler to SIG_IGN");
    signal(SIGCHLD, SIG_IGN);
}

pid_t Monitor::startRootfsPrepaer(util::ReadableFD config, util::FD socket,
                                  util::ReadableFD rootfsRead,
                                  util::WriteableFD rootfsWrite,
                                  util::WriteableFD monitor,
                                  util::ReadableFD initRead,
                                  util::WriteableFD initWrite)
{
    auto pid =
        util::exec("/proc/self/exe",
                   {
                       "ll-box-rootfs-preparer",
                       fmt::format("--config={}", config.__fd),
                       fmt::format("--socket={}", socket.__fd),
                       fmt::format("--rootfs_read={}", rootfsRead.__fd),
                       fmt::format("--rootfs_write={}", rootfsWrite.__fd),
                       fmt::format("--monitor_write={}", monitor.__fd),
                       fmt::format("--init_read={}", initRead.__fd),
                       fmt::format("--init_write={}", initWrite.__fd),
                   },
                   [&]() {
                       config.clear();
                       socket.clear();
                       rootfsRead.clear();
                       rootfsWrite.clear();
                       monitor.clear();
                       initRead.clear();
                       initWrite.clear();
                   });
    {
        int msg = -1;
        SPDLOG_TRACE(
            "waiting rootfs-preparer to request config uid/gid mappings");
        this->sync >> msg;
        SPDLOG_TRACE("done");
        if (msg != 0) {
            auto err = fmt::system_error(
                msg,
                "error occurred while waiting rootfs-preparer to request config uid/git mappings");
            SPDLOG_ERROR(err.what());
            throw err;
        }
    }

    OCI::util::setupIDMapping(pid, this->config.uidMappings.value(),
                              this->config.gidMappings.value());

    return pid;
}

} // namespace linglong::box::container
