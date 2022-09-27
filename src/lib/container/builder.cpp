#include "builder.h"
#include "oci/config.h"

namespace linglong::box::container {

using std::move;

Builder::Builder(const std::string &containerID, util::FD pathToBundle, nlohmann::json configJson,
                 util::FD containerWorkingDir, util::FD socketFD)
    : container(move(configJson), move(pathToBundle))
{
    SPDLOG_TRACE("linglong::box::container::Builder::Builder called");
    SPDLOG_TRACE("[containerID=\"{}\", containerWorkingDir=\"{}\" (fd={}), socketFD={}]", containerID,
                 containerWorkingDir.path(), containerWorkingDir.__fd, socketFD.__fd);
}

void Builder::Create()
{
    SPDLOG_TRACE("linglong::box::container::Builder::Create called");
}

} // namespace linglong::box::container
