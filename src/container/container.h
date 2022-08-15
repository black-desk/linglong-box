#ifndef LINGLONG_BOX_SRC_CONTAINER_CONTAINER_H
#define LINGLONG_BOX_SRC_CONTAINER_CONTAINER_H

#include <filesystem>
#include <map>
#include <optional>

#include <nlohmann/json.hpp>

#include "oci/config.h"
#include "oci/runtime.h"

namespace linglong {
class Container
{
public:
    Container(const std::string &containerID, const std::filesystem::path &bundle, const nlohmann::json &configJson,
              const std::filesystem::path &workingPath);
    void Create();

    struct OCI::Runtime::State state;
    std::string ID;
    OCI::Config config;
    std::filesystem::path workingPath;
    std::filesystem::path bundlePath;
};
} // namespace linglong

#endif /* LINGLONG_BOX_SRC_CONTAINER_H */
