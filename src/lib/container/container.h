#ifndef LINGLONG_BOX_SRC_LIB_CONTAINER_CONTAINER_H_
#define LINGLONG_BOX_SRC_LIB_CONTAINER_CONTAINER_H_

#include "oci/state.h"
#include "util/common.h"
#include "oci/config.h"
#include "util/fd.h"

namespace linglong::box::container {

struct Container : public util::NonCopyable {
    State state;
    util::FD __bundle;
    OCI::Config __config;

    Container(nlohmann::json config, util::FD pathToBundle);
};

} // namespace linglong::box::container

#endif
