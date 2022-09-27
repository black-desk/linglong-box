#include "builder.h"

namespace linglong::box::container {

Builder::Builder(const std::string &containerID, util::FD pathToBundle, nlohmann::json configJson,
                 util::FD containerWorkingDir, util::FD socketFD)
{
    SPDLOG_TRACE("linglong::box::container::Builder::Builder called");
    SPDLOG_TRACE("[containerID=\"{}\", pathToBundle=\"{}\" (fd={}), containerWorkingDir=\"{}\" (fd={}), socketFD={}]",
                 containerID, pathToBundle.path(), pathToBundle.__fd, containerWorkingDir.path(),
                 containerWorkingDir.__fd, socketFD.__fd);
    SPDLOG_TRACE("oci config:\n{}", configJson.dump(4));

    // TODO
}

} // namespace linglong::box::container
