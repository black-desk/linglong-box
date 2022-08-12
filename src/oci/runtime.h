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
    void Create(std::string containerID, std::string pathToBundle);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#start
    void Start(std::string containerID);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#kill
    void Kill(std::string containerID);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#delete
    void Delete(std::string containerID);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#query-state
    nlohmann::json State(std::string containerID);

    // NOT STANDARD

    std::vector<std::string> List();
    int Exec(std::string containerID, std::string pathToProcess, bool detach = false);
    int Exec(std::string containerID, std::vector<std::string> commandToExec, bool detach = false);

private:
    std::filesystem::path workingDir;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(struct Runtime::State, ociVersion, id, status, pid, bundle,
                                                annotations);
} // namespace linglong::OCI
#endif
