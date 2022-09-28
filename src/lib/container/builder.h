#ifndef LINGLONG_BOX_SRC_LIB_CONTAINER_BUILDER_H_
#define LINGLONG_BOX_SRC_LIB_CONTAINER_BUILDER_H_

#include <memory>

#include "lib/oci/config.h"
#include "lib/oci/state.h"
#include "util/pipe.h"

namespace linglong::box::container {

struct Builder {
    Builder(const std::string &containerID, util::FD pathToBundle, nlohmann::json configJson,
            util::FD containerWorkingDir, util::FD socketFD);
    ~Builder();
    void Create();
    void AfterCreated();
    std::optional<util::WriteableFD> monitorPipe;
    std::optional<util::WriteableFD> rootfsPipe;
    std::optional<util::WriteableFD> initPipe;
    std::optional<util::ReadableFD> pipe;

    State state;
    util::FD socket;
    std::optional<util::FD> workingDir;
    OCI::Config config;

    pid_t startMonitor(util::FD config, util::FD socket, util::WriteableFD runtimeWrite, util::ReadableFD monitorRead,
                       util::WriteableFD monitorWrite, util::ReadableFD rootfsRead, util::WriteableFD rootfsWrite,
                       util::ReadableFD initRead, util::WriteableFD initWrite);
};

} // namespace linglong::box::container

#endif /* LINGLONG_BOX_SRC_LIB_CONTAINER_BUILDER_H_ */
