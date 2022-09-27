#ifndef LINGLONG_BOX_SRC_LIB_CONTAINER_CONTAINER_H_
#define LINGLONG_BOX_SRC_LIB_CONTAINER_CONTAINER_H_

#include "oci/state.h"
#include "util/common.h"

namespace linglong::box::container {

struct Container : public util::NonCopyable {
    State state;
};

} // namespace linglong::box::container

#endif
