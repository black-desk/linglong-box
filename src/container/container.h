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
    std::unique_ptr<OCI::Config> config;
    std::filesystem::path workingPath;
    std::filesystem::path bundlePath;
    Option option;
    const bool isRef;

private:
    struct Rootfs;
    struct Init;

    struct Monitor {
        Monitor(std::unique_ptr<Init>, std::unique_ptr<Rootfs>, std::unique_ptr<OCI::Config>);
        void create();

        std::unique_ptr<Init> server;
        std::unique_ptr<Rootfs> rootfs;
    };

    struct Rootfs {
        Rootfs(const OCI::Config::Annotations::Rootfs &config);
        void allocateStack();
        int cloneFlag;
        int stackSize;
        void *stackTop;
    };

    struct Init {
        Init(const std::filesystem::path &socketAddress);
        void Listen();
    };

    std::unique_ptr<Monitor> monitor;
};
} // namespace linglong

#endif /* LINGLONG_BOX_SRC_CONTAINER_H */
