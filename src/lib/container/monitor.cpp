#include <sys/prctl.h>
#include <sys/signal.h>

#include "monitor.h"
#include "util/fmt.h"


namespace linglong::box::container {

Monitor::Monitor(OCI::Config config, util::WriteableFD runtime,
                 util::WriteableFD rootfs, util::WriteableFD init,
                 util::ReadableFD monitor)
{
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

} // namespace linglong::box::container
