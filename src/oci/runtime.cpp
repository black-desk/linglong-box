#include <csignal>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "container/container.h"
#include "runtime.h"
#include "util/filesystem.h"
#include "util/lock.h"
#include "util/epoll.h"

namespace linglong {
namespace OCI {

static const std::string workingDirTemplate = "/run/user/{}/linglong";

Runtime::Runtime()
    : workingDir(fmt::format(workingDirTemplate, getuid()))
{
}

void Runtime::Create(const std::string &containerID, const std::string &pathToBundle)
{
    using linglong::util::FlockGuard;
    using linglong::util::fs::mkdirp;
    using linglong::util::fs::mkdir;
    using linglong::util::fs::rmrf;

    auto bundle = std::filesystem::absolute(pathToBundle);
    auto containerWorkingDir = this->workingDir / containerID;

    std::unique_ptr<Container> container;

    try {
        mkdirp(this->workingDir, 0755);

        {
            auto guard = FlockGuard(this->workingDir);

            mkdir(containerWorkingDir, 0755);

            std::ifstream configJsonFile;
            configJsonFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            configJsonFile.open(bundle / "config.json");

            nlohmann::json configJson;
            configJsonFile >> configJson;

            int socketFD = -1;

            socketFD = socket(AF_UNIX, SOCK_SEQPACKET, 0);
            if (socketFD != -1) {
                throw std::runtime_error(fmt::format("Failed to create socket: {}", strerror(errno)));
            }

            std::filesystem::path addr = this->workingDir / "socket";

            sockaddr_un name = {};
            name.sun_family = AF_UNIX;
            strncpy(name.sun_path, addr.c_str(), sizeof(name.sun_path) - 1);

            int ret = -1;
            ret = bind(socketFD, (const sockaddr *)&name, sizeof(name));
            if (ret == -1) {
                throw std::runtime_error(fmt::format("Failed to bind socket to \"{}\": {}", strerror(errno)));
            }

            container.reset(new Container(containerID, bundle, configJson, containerWorkingDir, socketFD,
                                          {
                                              false,
                                              false,
                                              false,
                                              false,
                                              (1024 * 1024),
                                              true,
                                              true,
                                          }));

            this->updateState(containerWorkingDir, container->state);
        }

        container->Create();

        {
            auto guard = FlockGuard(this->workingDir);
            this->updateState(containerWorkingDir, container->state);
        }

        container->monitor->sync << 0; // request run "createRuntime"

        int msg = 0;
        spdlog::debug("runtime: wait monitor to report hooks result");
        container->sync >> msg;
        spdlog::debug("runtime: done");
        if (msg != 0) {
            throw std::runtime_error("Error during waitting hooks finish");
        }
    } catch (const std::runtime_error &e) {
        auto guard = FlockGuard(this->workingDir);
        rmrf(containerWorkingDir);
        std::throw_with_nested(std::runtime_error(
            fmt::format("Failed to create container (name=\"{}\", bundle=\"{}\")", containerID, bundle)));
    }
}

void Runtime::updateState(const std::filesystem::path &containerWorkingDir, const struct State &state)
{
    auto stateJsonPath = containerWorkingDir / std::filesystem::path("state.json");
    std::ofstream stateJsonFile(stateJsonPath);
    if (!stateJsonFile.is_open()) {
        throw std::runtime_error(fmt::format("Failed to open file (\"{}\")", stateJsonPath));
    }

    nlohmann::json stateJson(state);
    stateJsonFile << stateJson;
}

void Runtime::Start(const std::string &containerID, const bool interactive)
{
    using linglong::util::FlockGuard;

    auto containerWorkingDir = this->workingDir / containerID;

    try {
        std::unique_ptr<ContainerRef> c;

        {
            auto lock = FlockGuard(this->workingDir);
            c.reset(new ContainerRef(containerWorkingDir));
        }

        c->Start();

        {
            auto lock = FlockGuard(this->workingDir);
            this->updateState(containerWorkingDir, c->state);
        }

        if (interactive) {
            // TODO: check stdin/stdout is a tty
            proxy(0, 1, c->socket.fd, c->terminalFD->fd);
        }

    } catch (...) {
        std::throw_with_nested(std::runtime_error("Command start failed"));
    }
}

void Runtime::Kill(const std::string &containerID, const int &sig)
{
    using linglong::util::FlockGuard;

    auto containerWorkingDir = this->workingDir / containerID;

    try {
        std::unique_ptr<ContainerRef> c;

        {
            auto lock = FlockGuard(this->workingDir);
            c.reset(new ContainerRef(containerWorkingDir));
        }

        c->Kill(sig);

    } catch (...) {
        std::throw_with_nested(std::runtime_error("Command kill failed"));
    }
}

void Runtime::Delete(const std::string &containerID)
{
    using linglong::util::FlockGuard;

    auto containerWorkingDir = this->workingDir / containerID;

    try {
        std::unique_ptr<ContainerRef> c;

        {
            auto lock = FlockGuard(this->workingDir);
            c.reset(new ContainerRef(containerWorkingDir));
        }

        c->Delete();

    } catch (...) {
        std::throw_with_nested(std::runtime_error("Command kill failed"));
    }
}

nlohmann::json Runtime::State(const std::string &containerID)
{
    // FIXME: TODO
}
std::vector<std::string> Runtime::List() const
{
    // FIXME: TODO
}

int Runtime::Exec(const std::string &containerID, const std::string &pathToProcess, const bool detach)
{
    // FIXME: TODO
}

int Runtime::Exec(const std::string &containerID, const std::vector<std::string> &commandToExec, const bool detach)
{
    // FIXME: TODO
}

static void copy(int in, int out)
{
    while (true) {
        int nread = copy_file_range(in, nullptr, out, nullptr, 4096, 0);
        if (nread < 0) {
            if (errno == EIO)
                return;
            throw std::runtime_error(fmt::format("Failed to copy_file_range: {}", strerror(errno)));
        }
        continue;
    }
}

void Runtime::proxy(int in, int out, int notify, int target)
{
    util::Epoll epoll;
    epoll.add(in, [in, target](util::Epoll &epoll, const epoll_event &event) { copy(in, target); });
    epoll.add(target, [out, target](util::Epoll &epoll, const epoll_event &event) { copy(target, out); });
    epoll.add(notify, [](util::Epoll &epoll, const epoll_event &event) { epoll.end(); });
    epoll.run();
}

} // namespace OCI
} // namespace linglong
