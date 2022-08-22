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
              const std::filesystem::path &workingPath, int createContainerSocket, const Option &option = Option());
    void Create();
    void Start();
    void Kill();
    void Exec(const OCI::Config::Process &p);

    void runCreateHooks();
    void detach();

    struct OCI::Runtime::State state;
    std::string ID;
    std::unique_ptr<OCI::Config> config;
    std::filesystem::path workingPath;
    std::filesystem::path bundlePath;
    Option option;
    const bool isRef;

private:
    int hooksSocket;

    struct Rootfs;
    struct PID1;

    struct Monitor {
        Monitor(const std::filesystem::path &workingPath, std::unique_ptr<OCI::Config>, int createSocket,
                int hookSocket);

        void run();

        void initSignalHandler();

        std::filesystem::path workingPath;
        std::unique_ptr<PID1> pid1;
        std::unique_ptr<Rootfs> rootfs;
    };

    struct Rootfs {
        Rootfs(const OCI::Config::Annotations::Rootfs &config);
        void allocateStack();
        void setup(PID1 &pid1);

        int cloneFlag;
        int stackSize;
        void *stackTop;
    };

    struct PID1 {
        PID1(const OCI::Config &config, const std::filesystem::path &socketAddress);
        ~PID1();
        void Listen();

        int cloneFlag;
        int stackSize;
        void *stackTop;
        int createContainerSocket;
        int poststartSocket;
        int writeIDMappingSocket;
    };

    std::unique_ptr<Monitor> monitor;
};
} // namespace linglong

#endif /* LINGLONG_BOX_SRC_CONTAINER_H */
