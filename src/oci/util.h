#ifndef LINGLONG_BOX_SRC_OCI_UTIL_H
#define LINGLONG_BOX_SRC_OCI_UTIL_H

#include "config.h"

namespace linglong::OCI {

pid_t execProcess(const OCI::Config::Process &process);
void execHook(const linglong::OCI::Config::Hooks::Hook &hook);
} // namespace linglong::OCI::util

#endif
