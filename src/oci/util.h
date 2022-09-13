#ifndef LINGLONG_BOX_SRC_OCI_UTIL_H
#define LINGLONG_BOX_SRC_OCI_UTIL_H

#include "config.h"

namespace linglong::OCI {

pid_t execProcess(const OCI::Config::Process &process);
} // namespace linglong::OCI::util

#endif
