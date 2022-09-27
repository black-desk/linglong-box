#include "container.h"

namespace linglong::box::container {

using std::move;

Container::Container(nlohmann::json config, util::FD pathToBundle)
    : __bundle(move(pathToBundle))
    , __config(move(config.get<OCI::Config>()))
{
    SPDLOG_TRACE("linglong::box::container::Container::Container called");
    SPDLOG_TRACE("[bundle=\"{}\" (fd={})]", this->__bundle.path(), this->__bundle.__fd);
    SPDLOG_TRACE("oci config:\n{}", config.dump(4));

    try {
        this->__config.parse(this->__bundle.path());
        SPDLOG_TRACE("parsed oci config:\n{}", nlohmann::json(this->__config).dump(4));

    } catch (...) {
        auto err = fmt::system_error(errno, "failed to construct container");
        SPDLOG_ERROR(err.what());
        std::throw_with_nested(err);
    }
}

} // namespace linglong::box::container
