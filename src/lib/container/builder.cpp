#include "builder.h"
#include "oci/config.h"
#include "util/exec.h"
#include <sys/mman.h>

namespace linglong::box::container {

using std::move;

Builder::Builder(const std::string &containerID, util::FD pathToBundle, nlohmann::json configJson,
                 util::FD containerWorkingDir, util::FD socketFD)
    : socket(move(socketFD))
    , config(configJson.get<OCI::Config>())
    , workingDir(move(containerWorkingDir))
{
    auto bundle = pathToBundle.path();
    SPDLOG_TRACE("linglong::box::container::Builder::Builder called");
    SPDLOG_TRACE("[containerID=\"{}\", pathToBundle=\"{}\" (fd={}), containerWorkingDir=\"{}\" (fd={}), socketFD={}]",
                 containerID, bundle, pathToBundle.__fd, workingDir.value().path(), workingDir.value().__fd,
                 socket.__fd);
    SPDLOG_TRACE("oci config.json:\n{}", configJson.dump(4));

    config.parse(pathToBundle.path());

    SPDLOG_TRACE("parsed oci config.json:\n{}", nlohmann::json(config).dump(4));

    state.ociVersion = config.ociVersion;
    state.bundle = bundle;
    state.id = containerID;
}

void Builder::Create()
{
    SPDLOG_TRACE("linglong::box::container::Builder::Create called");

    try {
        pid_t monitorPid;

        {
#define CREATE_PIPE(name) auto [name##Read, name##Write] = util::pipe();

            CREATE_PIPE(runtime);
            CREATE_PIPE(monitor);
            CREATE_PIPE(rootfs);
            CREATE_PIPE(init);

#undef CREATE_PIPE
            auto ret = memfd_create("config.json", 0);
            if (ret == -1) {
                auto err = fmt::system_error(errno, "failed to create memfd for parsed config.json");
                SPDLOG_ERROR(err.what());
                throw err;
            }
            auto configFD = util::WriteableFD(ret);
            configFD << nlohmann::json(config).dump();

            this->pipe = std::move(runtimeRead);
            this->monitorPipe = monitorWrite.dup();
            this->initPipe = initWrite.dup();
            this->rootfsPipe = rootfsWrite.dup();

            monitorPid = this->startMonitor(std::move(configFD), std::move(this->socket), std::move(runtimeWrite),
                                            std::move(monitorRead), std::move(monitorWrite), std::move(rootfsRead),
                                            std::move(rootfsWrite), std::move(initRead), std::move(initWrite));
        }

        SPDLOG_DEBUG("pid of monitor={}", monitorPid);
        this->state.annotations = State::Annotations {monitorPid};

        SPDLOG_DEBUG("waiting rootfs-preparer report pid of init");
        this->pipe.value() >> this->state.pid;
        SPDLOG_DEBUG("done, pid of init={}", this->state.pid);

    } catch (...) {
        auto err = std::runtime_error("builder failed to create container");
        SPDLOG_ERROR(err.what());
        std::throw_with_nested(err);
    }
}

void Builder::AfterCreated()
{
    SPDLOG_TRACE("request monitor/init to run \"prestart/createRuntime/createContainer\"");
    assert(this->monitorPipe.has_value());
    this->monitorPipe.value() << 0;
    SPDLOG_TRACE("done");

    int msg = -1;
    SPDLOG_DEBUG("waiting monitor to report \"prestart/createRuntime/createContainer\" hooks result");
    assert(this->pipe.has_value());
    this->pipe.value() >> msg;
    SPDLOG_DEBUG("done");

    if (msg < 0) {
        auto err = fmt::system_error(-msg, "failed to run \"prestart/createRuntime/createContainer\" hooks");
        SPDLOG_ERROR(err.what());
        throw err;
    }

    this->workingDir.reset();
}

Builder::~Builder()
{
    try {
        if (this->workingDir.has_value()) {
            std::filesystem::remove_all(this->workingDir->path());
        }
    } catch (...) {
        SPDLOG_CRITICAL("UNEXPECTED error during clean up");
    }
}

pid_t Builder::startMonitor(util::FD config, util::FD socket, util::WriteableFD runtimeWrite,
                            util::ReadableFD monitorRead, util::WriteableFD monitorWrite, util::ReadableFD rootfsRead,
                            util::WriteableFD rootfsWrite, util::ReadableFD initRead, util::WriteableFD initWrite)
{
    return util::exec(std::filesystem::read_symlink("/proc/self/exe"),
                      {
                          "ll-box-monitor",
                          fmt::format("--config={}", config.__fd),
                          fmt::format("--socket={}", socket.__fd),
                          fmt::format("--runtime_write={}", runtimeWrite.__fd),
                          fmt::format("--monitor_read={}", monitorRead.__fd),
                          fmt::format("--monitor_write={}", monitorWrite.__fd),
                          fmt::format("--rootfs_read={}", rootfsRead.__fd),
                          fmt::format("--rootfs_write={}", rootfsWrite.__fd),
                          fmt::format("--init_read={}", initRead.__fd),
                          fmt::format("--init_write={}", initWrite.__fd),
                      },
                      [&]() {
                          config.clear();
                          socket.clear();
                          runtimeWrite.clear();
                          monitorRead.clear();
                          monitorWrite.clear();
                          rootfsRead.clear();
                          rootfsWrite.clear();
                          initRead.clear();
                          initWrite.clear();
                      });
}

} // namespace linglong::box::container
