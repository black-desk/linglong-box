#ifndef LINGLONG_BOX_SRC_OCI_RUNTIME_H
#define LINGLONG_BOX_SRC_OCI_RUNTIME_H

#include <string>

#include "config.h"

namespace linglong::OCI {

// https://github.com/opencontainers/runtime-spec/blob/main/runtime.md
class Runtime
{
public:
    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#state
    struct State {
        std::string ociVersion;
        std::string id;
        std::string status;
        int pid;
        std::string bundle;
        std::optional<Config::Annotations> annotations;
    };

    Runtime();

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#create
    void Create(const std::string &containerID, const std::string &pathToBundle);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#start
    void Start(const std::string &containerID, const bool interactive = false);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#kill
    void Kill(const std::string &containerID);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#delete
    void Delete(const std::string &containerID);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#query-state
    nlohmann::json State(const std::string &containerID);

    // NOT STANDARD

    std::vector<std::string> List() const;
    int Exec(const std::string &containerID, const std::string &pathToProcess, const bool detach = false);
    int Exec(const std::string &containerID, const std::vector<std::string> &commandToExec, const bool detach = false);

private:
    std::filesystem::path workingDir;
    void updateState(const std::filesystem::path &containerWorkingDir, const struct State &state);
    void proxy(int in, int out, int notify,int target);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(struct Runtime::State, ociVersion, id, status, pid, bundle,
                                                annotations);
} // namespace linglong::OCI
#endif
