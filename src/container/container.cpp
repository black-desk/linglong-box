

#include "container.h"

namespace linglong {
Container::Container(const std::string &containerID, const nlohmann::json &configJson,
                     const std::filesystem::path &workingPath)
    : ID(containerID)
    , workingPath(workingPath)
{
    configJson.get_to(this->config);
    this->config.parse();
}

void Container::Create()
{
}
} // namespace linglong
