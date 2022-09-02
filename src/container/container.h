#ifndef LINGLONG_BOX_SRC_CONTAINER_CONTAINER_H
#define LINGLONG_BOX_SRC_CONTAINER_CONTAINER_H

#include <filesystem>
#include <map>
#include <optional>

#include <nlohmann/json.hpp>

#include "oci/config.h"
#include "oci/runtime.h"
#include "util/fd.h"
#include "util/sync.h"

namespace linglong {

void configIDMapping(pid_t target, const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &uidMappings,
                     const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &gidMappings);
void execHook(const linglong::OCI::Config::Hooks::Hook &hook);

void makeSureParentSurvive(pid_t ppid = 0) noexcept;

void ignoreParentDie() noexcept;

int _(void *arg);

std::string dbusProxyPath();

std::string fuseOverlayfsPath();

std::vector<std::string> environPassThrough();

struct doMountOption {
    bool ignoreError;
    bool resolveRealPath;
    bool fallback;
};

void doMount(const linglong::OCI::Config::Mount &, const util::FD &root, const std::filesystem::path &rootpath,
             const doMountOption &opt = {false, true, true});

class Container
{
public:
    struct Option {
        bool ProcessAsEntrypoint;
        bool EnableHooks;
        bool SetupSystemdScope;
        bool IgnoreMountFail;
        uint StackSize;
        bool CreateDefaultDevice;
        bool LinkLFS;
    };

    struct Monitor {
        Monitor(Container *const container);
        void run();

        Container *const container;
        util::Pipe sync;
        pid_t pid;

        void init(pid_t ppid) noexcept;
        void exec();
    };

    struct Rootfs {
        Rootfs(Container *const container);
        void run();
        pid_t pid;

        Container *const container;
        util::Pipe sync;

        int unshareFlag;

        void init(pid_t ppid) noexcept;
        void clear();
        void prepareRootfs();
    };

    struct Init {
        Init(Container *const container, int socket);
        void run();
        pid_t pid;

        Container *const container;
        util::Pipe sync;
        int socket;
        std::unique_ptr<util::FD> terminalFD;

        void init(pid_t ppid) noexcept;
        void setupContainer();

        void setNS();
        void setMounts();
        void setLink();
        void setCgroup();
        void setSeccomp();
        void setDevices();
        void setConsole();

        void pivotRoot();
        void waitStart();
        void execProcess();

        void clear(util::Pipe &conn);
    };

    Container(const std::string &containerID, const std::filesystem::path &bundle, const nlohmann::json &configJson,
              const std::filesystem::path &workingPath, int sync, const Option &option = Option());

    void Create();
    struct OCI::Runtime::State state;
    std::string ID;
    std::unique_ptr<OCI::Config> config;
    std::filesystem::path bundlePath;
    Option option;
    std::unique_ptr<util::FD> containerRoot;

    util::Pipe sync;

    std::unique_ptr<Monitor> monitor;
    std::unique_ptr<Rootfs> rootfs;
    std::unique_ptr<Init> init;
};

struct ContainerRef {
    ContainerRef(const std::filesystem::path &workingPath);
    void Start();

    util::FD connect();

    std::filesystem::path workingPath;
    struct OCI::Runtime::State state;
    util::Pipe socket;
    util::FD terminalFD;
};

} // namespace linglong

#endif /* LINGLONG_BOX_SRC_CONTAINER_H */
