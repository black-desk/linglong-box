#ifndef LINGLONG_BOX_SRC_OCI_UTIL_H
#define LINGLONG_BOX_SRC_OCI_UTIL_H

#include "lib/oci/config.h"

namespace linglong::box::OCI::util {

pid_t execProcess(const Config::Process &process);
void execHook(const Config::Hooks::Hook &hook);
void setupIDMapping(pid_t pid, const std::vector<Config::IDMapping> &uidMapping,
                    const std::vector<Config::IDMapping> &gidMapping);
} // namespace linglong::box::OCI

#endif
