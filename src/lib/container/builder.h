#ifndef LINGLONG_BOX_SRC_LIB_CONTAINER_BUILDER_H_
#define LINGLONG_BOX_SRC_LIB_CONTAINER_BUILDER_H_

#include <memory>

#include "container.h"
#include "util/pipe.h"

namespace linglong::box::container {

struct Builder : virtual public util::Singleton<Builder> {
    Builder(const std::string &containerID, util::FD pathToBundle, nlohmann::json configJson,
            util::FD containerWorkingDir, util::FD socketFD);
    void Create();
    Container container;
    std::unique_ptr<util::PipeWriteEnd> monitorPipe;
    std::unique_ptr<util::PipeWriteEnd> rootfsPipe;
    std::unique_ptr<util::PipeWriteEnd> initPipe;
    std::unique_ptr<util::PipeReadEnd> pipe;
};

} // namespace linglong::box::container

#endif /* LINGLONG_BOX_SRC_LIB_CONTAINER_BUILDER_H_ */
