#ifndef LINGLONG_BOX_SRC_OCI_RUNTIME_H
#define LINGLONG_BOX_SRC_OCI_RUNTIME_H

#include <filesystem>
#include <string>
#include <vector>

#include "state.h"
#include "util/fd.h"

namespace linglong::box::OCI {

// https://github.com/opencontainers/runtime-spec/blob/main/runtime.md
class Runtime
{
public:
    Runtime();

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#create
    void Create(const std::string &containerID, util::FD bundlePath);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#start
    void Start(const std::string &containerID, const bool interactive, const std::string &consoleSocket,
               const int extraFDs, const bool boxAsInit);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#kill
    void Kill(const std::string &containerID, const int &signal);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#delete
    void Delete(const std::string &containerID);

    // https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#query-state
    std::string State(const std::string &containerID);

    // NOT STANDARD
    std::vector<std::string> List() const;
    // NOT STANDARD
    void Exec(const std::string &containerID, util::FD processJsonPath, const bool interactive,
              const std::string &consoleSocket, const int extraFDs);
    // NOT STANDARD
    void Exec(const std::string &containerID, const std::vector<std::string> &commandToExec, const bool interactive,
              const std::string &consoleSocket, const int extraFDs);

private:
    util::FD workingDir;
    void updateState(const std::string &containerID, const struct State &state);
    void proxy(int in, int out, int notify, int target);
};

} // namespace linglong::box::OCI
#endif
