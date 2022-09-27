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

    enum Status { Creating, Created, Running, Stopped };

    std::string ociVersion;
    std::string id;
    Status status;
    int pid;
    std::string bundle;
    std::optional<Annotations> annotations;
};

#define JSON_DISABLE_ENUM_SERIALIZATION 1
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(State::Annotations, monitorPid);
NLOHMANN_JSON_SERIALIZE_ENUM(State::Status, {
                                                {State::Status::Creating, "creating"},
                                                {State::Status::Created, "created"},
                                                {State::Status::Running, "running"},
                                                {State::Status::Stopped, "stopped"},
                                            });
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(State, ociVersion, id, status, pid, bundle, annotations);

} // namespace linglong::box

#endif /* LINGLONG_BOX_SRC_OCI_STATE_H_ */
