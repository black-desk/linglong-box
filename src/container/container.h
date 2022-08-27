#ifndef LINGLONG_BOX_SRC_CONTAINER_CONTAINER_H
#define LINGLONG_BOX_SRC_CONTAINER_CONTAINER_H

#include <filesystem>
#include <map>
#include <optional>

#include <nlohmann/json.hpp>

#include "oci/config.h"
#include "oci/runtime.h"
#include "util/sync.h"

namespace linglong {

void configIDMapping(pid_t target, const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &uidMappings,
                     const std::optional<std::vector<linglong::OCI::Config::IDMapping>> &gidMappings);
void execHook(const linglong::OCI::Config::Hooks::Hook &hook);

void makeSureParentSurvive(pid_t ppid) noexcept;

void ignoreParentDie();

int _(void *arg);

std::string dbusProxyPath();

std::string fuseOverlayfsPath();

std::vector<std::string> environPassThrough();

void doMount(const linglong::OCI::Config::Mount &);

class Container
{
public:
    struct Option {
        bool ProcessAsEntrypoint;
        bool EnableHooks;
        bool SetupSystemdScope;
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
        void exec();
        void prepareRootfs();
    };

    struct Init {
        Init(Container *const container);
        void run();
        pid_t pid;

        Container *const container;
        util::Pipe sync;

        int cloneFlag;
        char *stackTop;

        void init(pid_t ppid) noexcept;
        void setupContainer();
    };

    Container(const std::string &containerID, const std::filesystem::path &bundle, const nlohmann::json &configJson,
              const std::filesystem::path &workingPath, int sync, const Option &option = Option());

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

    util::Pipe sync;

    std::unique_ptr<Monitor> monitor;
    std::unique_ptr<Rootfs> rootfs;
    std::unique_ptr<Init> init;
};
} // namespace linglong

#endif /* LINGLONG_BOX_SRC_CONTAINER_H */
