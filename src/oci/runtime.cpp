#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "container/container.h"
#include "runtime.h"
#include "util/filesystem.h"
#include "util/lock.h"

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
            proxy(0, 1, c->socket.fd, c->terminalFD.fd);
        }

    } catch (...) {
        std::throw_with_nested(std::runtime_error("Command start failed"));
    }
}

void Runtime::Kill(const std::string &containerID)
{
    // FIXME: TODO
}
void Runtime::Delete(const std::string &containerID)
{
    // FIXME: TODO
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

inline void epoll_ctl_add(int epfd, int fd)
{
    static epoll_event ev = {};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    if (ret != 0) {
        throw std::runtime_error(fmt::format("Failed to fd={} to epoll: {}", fd, strerror(errno)));
    }
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
    auto epfd = util::FD(epoll_create(1));
    epoll_ctl_add(epfd.fd, in);
    epoll_ctl_add(epfd.fd, target);
    epoll_ctl_add(epfd.fd, notify);
    for (;;) {
        struct epoll_event events[10];
        int event_cnt = epoll_wait(epfd.fd, events, 5, -1);
        for (int i = 0; i < event_cnt; i++) {
            const auto &event = events[i];
            if (event.data.fd == in) {
                copy(in, target);
            } else if (event.data.fd == target) {
                copy(target, out);
            } else {
                break;
            }
        }
    }
}

} // namespace OCI
} // namespace linglong
