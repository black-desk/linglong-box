#ifndef LINGLONG_BOX_SRC_LIB_CONTAINER_MONITOR_H_
#define LINGLONG_BOX_SRC_LIB_CONTAINER_MONITOR_H_

#include "lib/oci/config.h"
#include "util/pipe.h"

namespace linglong::box::container {

struct Monitor {
    util::ReadableFD sync;
    OCI::Config config;

    Monitor(OCI::Config config, util::WriteableFD runtime,
            util::WriteableFD rootfs, util::WriteableFD init,
            util::ReadableFD monitor);
    pid_t startRootfsPrepaer(util::ReadableFD config, util::FD socket,
                             util::ReadableFD rootfsRead,
                             util::WriteableFD rootfsWrite,
                             util::WriteableFD monitor,
                             util::ReadableFD initRead,
                             util::WriteableFD initWrite);
    void handleHooks();
    ~Monitor();
};

} // namespace linglong::box::container

#endif
