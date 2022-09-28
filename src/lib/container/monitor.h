#ifndef LINGLONG_BOX_SRC_LIB_CONTAINER_MONITOR_H_
#define LINGLONG_BOX_SRC_LIB_CONTAINER_MONITOR_H_

#include "lib/oci/config.h"
#include "util/pipe.h"

namespace linglong::box::container {

struct Monitor {
    Monitor(OCI::Config config, util::WriteableFD runtime,
            util::WriteableFD rootfs, util::WriteableFD init,
            util::ReadableFD monitor);
    void startRootfsPrepaer();
    void handleHooks();
    ~Monitor();
};

} // namespace linglong::box::container

#endif
