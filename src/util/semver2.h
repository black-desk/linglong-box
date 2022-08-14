#ifndef LINGLONG_BOX_SRC_UTIL_SEMVER2
#define LINGLONG_BOX_SRC_UTIL_SEMVER2

#include <string>

namespace linglong::util {

struct SemVer {
    SemVer(std::string s);
    bool isCompatibleWith(const SemVer &s);

    unsigned long long major, minor, patch;
    std::string prerelease, buildmetadata;
};

} // namespace linglong::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_SEMVER2 */
