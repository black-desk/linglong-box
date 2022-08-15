

#include "container.h"

namespace linglong {
Container::Container(const std::string &containerID, const std::filesystem::path &bundle,
                     const nlohmann::json &configJson, const std::filesystem::path &workingPath)
    : ID(containerID)
    , workingPath(workingPath)
    , bundlePath(bundle)
{
    configJson.get_to(this->config);
    this->config.parse(bundlePath);
}

void Container::Create()
{
}
} // namespace linglong
