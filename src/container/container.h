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
    struct Option {
        bool ProcessAsEntrypoint;
    };

    Container(const std::string &containerID, const std::filesystem::path &bundle, const nlohmann::json &configJson,
              const std::filesystem::path &workingPath, const Option &option = Option());
    void Create();
    void Start();
    void Kill();
    void Exec(const OCI::Config::Process &p);

    struct OCI::Runtime::State state;
    std::string ID;
    OCI::Config config;
    std::filesystem::path workingPath;
    std::filesystem::path bundlePath;
    Option option;

private:
    struct Main {
        struct LocalServer {
            LocalServer(const std::filesystem::path &socketAddress);
            void Listen();
        };

        struct Rootfs {
            Rootfs(const OCI::Config::Annotations::Rootfs &config);
        };

        Main(std::unique_ptr<LocalServer>, std::unique_ptr<Rootfs>);
        int create();

        std::unique_ptr<LocalServer> server;
        std::unique_ptr<Rootfs> rootfs;
    };
    std::unique_ptr<Main> main;
};
} // namespace linglong

#endif /* LINGLONG_BOX_SRC_CONTAINER_H */
