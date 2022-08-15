#include "semver2.h"
#include "fmt/format.h"
#include "util/exception.h"

#include <regex>

static std::regex rgx(
    R"(^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$)");

namespace linglong::util {
SemVer::SemVer(std::string s)
{
    std::smatch matches;

    std::regex_search(s, matches, rgx);

    if (matches.size() != 6) {
        throw RuntimeError(fmt::format("Fail to parse SemVer from \"{}\"", s));
    }

    this->major = std::stoull(matches[1].str());
    this->minor = std::stoull(matches[2].str());
    this->patch = std::stoull(matches[3].str());
    this->prerelease = matches[4].str();
    this->buildmetadata = matches[5].str();
}

bool SemVer::isCompatibleWith(const SemVer &s)
{
    // FIXME: this just a workround, as ll-box semver always equal to 1.0.2, this will be fine.
    return this->major == s.major && this->minor > s.minor;
}

} // namespace linglong::util
