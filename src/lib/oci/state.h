#ifndef LINGLONG_BOX_SRC_OCI_STATE_H_
#define LINGLONG_BOX_SRC_OCI_STATE_H_

#include <optional>
#include <string>

#include "serializer.h"

namespace linglong::box {

// https://github.com/opencontainers/runtime-spec/blob/main/runtime.md#state
struct State {
    struct Annotations {
        int monitorPid;
    };

    std::string ociVersion;
    std::string id;
    std::string status;
    int pid;
    std::string bundle;
    std::optional<Annotations> annotations;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(State::Annotations, monitorPid);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(State, ociVersion, id, status, pid, bundle, annotations);

} // namespace linglong::box

#endif /* LINGLONG_BOX_SRC_OCI_STATE_H_ */
