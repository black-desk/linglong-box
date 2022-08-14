#include "config.h"

#include <spdlog/spdlog.h>

#include "fmt/core.h"
#include "util/semver2.h"

using linglong::util::SemVer;

static const SemVer llboxOciSemVersion("1.0.2");

void linglong::OCI::Config::parse(const std::filesystem::path &bundlePath)
{
    {
        util::SemVer ociSemVersion(this->ociVersion);
        if (!ociSemVersion.isCompatibleWith(llboxOciSemVersion)) {
            spdlog::warn("OCI Version of config.json \"{}\" not compatible with ll-box", ociVersion);
        }
    }

    this->root.readonly = this->root.readonly.value_or(false);
    if (this->root.path.is_relative()) {
        this->root.path = bundlePath / this->root.path;
    }

    {
        for (auto &mount : this->mounts.value_or(std::vector<Mount>())) { }
    }
}
