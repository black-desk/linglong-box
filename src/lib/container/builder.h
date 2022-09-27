#ifndef LINGLONG_BOX_SRC_LIB_CONTAINER_BUILDER_H_
#define LINGLONG_BOX_SRC_LIB_CONTAINER_BUILDER_H_

#include <memory>

#include "oci/config.h"
#include "oci/state.h"
#include "util/pipe.h"

namespace linglong::box::container {

struct Builder {
    Builder(const std::string &containerID, util::FD pathToBundle, nlohmann::json configJson,
            util::FD containerWorkingDir, util::FD socketFD);
    void Create();
    std::optional<util::PipeWriteEnd> monitorPipe;
    std::optional<util::PipeWriteEnd> rootfsPipe;
    std::optional<util::PipeWriteEnd> initPipe;
    std::optional<util::PipeReadEnd> pipe;

    State state;
    util::FD socket;
    OCI::Config config;

    void startMonitor(util::FD config, util::FD socket, util::PipeWriteEnd runtimeWrite, util::PipeReadEnd monitorRead,
                      util::PipeWriteEnd monitorWrite, util::PipeReadEnd rootfsRead, util::PipeWriteEnd rootfsWrite,
                      util::PipeReadEnd initRead, util::PipeWriteEnd initWrite);
};

} // namespace linglong::box::container

#endif /* LINGLONG_BOX_SRC_LIB_CONTAINER_BUILDER_H_ */
