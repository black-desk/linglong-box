#include "builder.h"
#include "oci/config.h"

namespace linglong::box::container {

using std::move;

Builder::Builder(const std::string &containerID, util::FD pathToBundle, nlohmann::json configJson,
                 util::FD containerWorkingDir, util::FD socketFD)
    : socket(move(socketFD))
    , config(configJson.get<OCI::Config>())
{
    auto bundle = pathToBundle.path();
    SPDLOG_TRACE("linglong::box::container::Builder::Builder called");
    SPDLOG_TRACE("[containerID=\"{}\", pathToBundle=\"{}\" (fd={}), containerWorkingDir=\"{}\" (fd={}), socketFD={}]",
                 containerID, bundle, pathToBundle.__fd, containerWorkingDir.path(), containerWorkingDir.__fd,
                 socket.__fd);
    SPDLOG_TRACE("oci config.json:\n{}", configJson.dump(4));

    config.parse(pathToBundle.path());

    SPDLOG_TRACE("parsed oci config.json:\n{}", nlohmann::json(config).dump(4));

    state.ociVersion = config.ociVersion;
    state.bundle = pathToBundle.path();
}

void Builder::Create()
{
    SPDLOG_TRACE("linglong::box::container::Builder::Create called");

    try {
#define CREATE_PIPE(name) auto [name##Read, name##Write] = util::pipe();

        CREATE_PIPE(runtime);
        CREATE_PIPE(monitor);
        CREATE_PIPE(rootfs);
        CREATE_PIPE(init);

#undef CREATE_PIPE

        auto [configFD, write] = util::pipe();
        write << nlohmann::json(config).dump();

        this->pipe = std::move(runtimeRead);
        this->monitorPipe = monitorWrite.dup();
        this->initPipe = initWrite.dup();
        this->rootfsPipe = rootfsWrite.dup();

        this->startMonitor(std::move(configFD), std::move(this->socket), runtimeWrite, monitorRead, monitorWrite,
                           rootfsRead, rootfsWrite, initRead, initWrite);

    } catch (...) {
        auto err = fmt::system_error(errno, "builder failed to create container");
        SPDLOG_ERROR(err.what());
        std::throw_with_nested(err);
    }
}

} // namespace linglong::box::container
