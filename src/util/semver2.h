#ifndef LINGLONG_BOX_SRC_UTIL_SEMVER2
#define LINGLONG_BOX_SRC_UTIL_SEMVER2

#include <string>
#include <regex>

#include "fmt/fmt.h"
#include "spdlog/spdlog.h"

namespace linglong::box::util {

struct SemVer {
    SemVer(std::string s)
    {
        SPDLOG_TRACE("linglong::box::util::SemVer::SemVer called");
        SPDLOG_TRACE("parsing semver2.0 from \"{}\"", s);

        static std::regex rgx(
            R"(^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$)");

        std::smatch matches;

        std::regex_search(s, matches, rgx);

        if (matches.size() != 6) {
            auto err = fmt::system_error(EINVAL, "fail to parse semver2.0 from \"{}\"", s);
            SPDLOG_ERROR(err.what());
            throw err;
        }

        this->major = std::stoull(matches[1].str());
        this->minor = std::stoull(matches[2].str());
        this->patch = std::stoull(matches[3].str());
        this->prerelease = matches[4].str();
        this->buildmetadata = matches[5].str();

        SPDLOG_TRACE("parsed major={}, minor={}, patch={}, prerelease={}, buildmetadata={}", this->major, this->minor,
                     this->patch, this->prerelease, this->buildmetadata);
    }

    bool isCompatibleWith(const SemVer &s)
    {
        // FIXME: this just a workround, as ll-box semver always equal to 1.0.2, this will be fine.
        return this->major == s.major && this->minor >= s.minor;
    }

    unsigned long long major, minor, patch;
    std::string prerelease, buildmetadata;
};

} // namespace linglong::box::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_SEMVER2 */
