#include "monitor.h"
#include "oci/config.h"
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
