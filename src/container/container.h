#ifndef LINGLONG_BOX_SRC_CONTAINER_CONTAINER_H
#define LINGLONG_BOX_SRC_CONTAINER_CONTAINER_H

#include <filesystem>
#include <map>
#include <optional>

#include <nlohmann/json.hpp>

#include "oci/config.h"
#include "oci/runtime.h"
#include "util/fd.h"

namespace linglong {

void configIDMapping(pid_t target, const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &uidMappings,
                     const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &gidMappings);

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

void doUmount(const linglong::OCI::Config::Mount &, const util::FD &root);

class ContainerBuilder
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
        Monitor(ContainerBuilder *const container);
        void run();

        ContainerBuilder *const container;
        util::Pipe sync;
        pid_t pid;

        void init(pid_t ppid) noexcept;
        void exec();
    };

    struct Rootfs {
        Rootfs(ContainerBuilder *const container);
        void run();
        pid_t pid;

        ContainerBuilder *const container;
        util::Pipe sync;

        int unshareFlag;

        void init(pid_t ppid);
        void clear();
        void prepareRootfs();
    };

    struct Init {
        Init(ContainerBuilder *const container, int socket);
        void run();
        pid_t pid;

        ContainerBuilder *const container;
        util::Pipe sync;
        int socket;
        std::shared_ptr<util::FD> terminalFD;
        std::map<pid_t, util::Pipe> map;

        void init(pid_t ppid) noexcept;
        void setupContainer();

        void setNS();
        void setMounts();
        void setLink();
        void setCgroup();
        void setSeccomp();
        void setDevices();

        void pivotRoot();
        void waitStart();
        pid_t execProcess(const OCI::Config::Process &p);

        void exec(util::Socket &sync, pid_t pid);
    };

    ContainerBuilder(const std::string &containerID, const std::filesystem::path &bundle, const nlohmann::json &configJson,
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

struct Container {
    Container(const std::filesystem::path &workingPath);
    void Start();
    void Kill(const int &sig);
    void Delete();
    void Exec();

    int connect();

    std::filesystem::path workingPath;
    struct OCI::Runtime::State state;
    util::Socket socket;
    std::shared_ptr<util::FD> terminalFD;
};

} // namespace linglong

#endif /* LINGLONG_BOX_SRC_CONTAINER_H */
