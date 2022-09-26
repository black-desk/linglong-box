// #include <csignal>
// #include <fcntl.h>
// #include <filesystem>
// #include <fstream>
// #include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
// #include <unistd.h>

// #include "fmt/format.h"
// #include "nlohmann/json.hpp"

// #include "lib/container/container.h"
// #include "util/fd.h"
// #include "util/lock.h"
// #include "util/epoll.h"

#include <filesystem>
#include <string>
#include <system_error>
#include <unistd.h>

#include "fmt/fmt.h"
#include "spdlog/spdlog.h"

#include "runtime.h"
#include "util/string.h"
#include "util/fmt.h"
#include "util/flock.h"

namespace linglong::box::OCI {

using nlohmann::json;
using fmt::format;
using std::string;
using util::FD;
using std::filesystem::path;
using std::move;

static const string workingDirTemplate = "/run/user/{}/linglong";

Runtime::Runtime()
    : workingDir([]() -> FD {
        auto uid = ::getuid();
        auto workingDir = uid ? format(workingDirTemplate, uid) : "/var/run/linglong";
        try {
            std::filesystem::create_directories(workingDir);
        } catch (...) {
            auto err = fmt::system_error(errno, "failed to create working directory \"{}\"", workingDir);
            SPDLOG_ERROR(err.what());
            std::throw_with_nested(err);
        }
        return FD(path(workingDir), O_PATH);
    }())
{
}

void Runtime::Create(const std::string &containerID, FD pathToBundle)
{
    SPDLOG_TRACE("linglong::box::OCI::Runtime::Create called");
    SPDLOG_TRACE("[containerID=\"{}\", pathToBundle=\"{}\"]", containerID, pathToBundle.path());

    using linglong::box::util::FLockGuard;

    {
        auto guard = FLockGuard(this->workingDir);

        auto containerWorkingDirPath = this->workingDir.path() / containerID;

        if (std::filesystem::exists(containerWorkingDirPath)) {
            auto err = fmt::system_error(EINVAL, "container \"{}\" existed", containerID);
            SPDLOG_ERROR(err.what());
            throw err;
        }

        try {
            std::filesystem::create_directory(containerWorkingDirPath);
        } catch (...) {
            auto err = fmt::system_error(errno, "failed to create container working directory \"{}\"",
                                         containerWorkingDirPath);
            SPDLOG_ERROR(err.what());
            std::throw_with_nested(err);
        }

        auto containerWorkingDir = this->workingDir.at(containerID);

        int ret;

        ret = socket(AF_UNIX, SOCK_SEQPACKET, 0);
        if (ret < 0) {
            auto err = fmt::system_error(errno, "failed to create socket(AF_UNIX, SOCK_SEQPACKET, 0)");
            SPDLOG_ERROR(err.what());
            throw err;
        }

        auto socketFD = FD(ret);

        {
            std::filesystem::path addr = containerWorkingDirPath / "socket";
            sockaddr_un name = {};
            name.sun_family = AF_UNIX;
            strncpy(name.sun_path, addr.c_str(), sizeof(name.sun_path) - 1);

            ret = bind(socketFD.__fd, (const sockaddr *)&name, sizeof(name));
            if (ret) {
                auto err = fmt::system_error(errno, "failed to bind socket to \"{}\"", addr);
                SPDLOG_ERROR(err);
                throw err;
            }
        }

        // TODO
    }

    // try {

    // std::unique_ptr<ContainerBuilder> builder;

    // {
    // std::ifstream configJsonFile;
    // configJsonFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    // configJsonFile.open(bundle / "config.json");

    // nlohmann::json configJson;
    // configJsonFile >> configJson;

    // builder.reset(new ContainerBuilder(containerID, bundle, configJson, containerWorkingDir, socketFD,
    // {
    // false,
    // false,
    // false,
    // false,
    // (1024 * 1024),
    // true,
    // true,
    // }));

    // this->updateState(containerWorkingDir, builder.container->state);
    // }

    // builder->Create();

    // {
    // auto guard = FlockGuard(this->workingDir);
    // this->updateState(containerWorkingDir, builder.container->state);
    // }

    // builder->monitorSync << 0; // request run "createRuntime"

    // int msg = 0;
    // SPDLOG_DEBUG("Waiting monitor to report hooks result");
    // builder->sync >> msg;
    // SPDLOG_DEBUG("Done");

    // if (msg != 0) {
    // throw std::runtime_error("Error during waitting hooks finish");
    // }
    // } catch (const std::runtime_error &e) {
    // auto guard = FlockGuard(this->workingDir);
    // std::filesystem::remove_all(containerWorkingDir);
    // std::throw_with_nested(std::runtime_error(
    // fmt::format("Failed to create container (name=\"{}\", bundle=\"{}\")", containerID, bundle)));
    // }
}

void Runtime::updateState(const std::filesystem::path &containerWorkingDir, const struct State &state)
{
    SPDLOG_TRACE("linglong::box::OCI::Runtime::updateState called ");
    SPDLOG_TRACE("[containerWorkingDir=\"{}\", state=\"{}\"]", containerWorkingDir, json(state).dump());

    // auto stateJsonPath = containerWorkingDir / "state.json";
    // std::ofstream stateJsonFile(stateJsonPath);
    // if (!stateJsonFile.is_open()) {
    // throw std::runtime_error(fmt::format("Failed to open file (\"{}\")", stateJsonPath));
    // }

    // nlohmann::json stateJson(state);
    // stateJsonFile << stateJson;
}

void Runtime::Start(const std::string &containerID, const bool interactive, const std::string &consoleSocket,
                    const int extraFDs, const bool boxAsInit)
{
    SPDLOG_TRACE("linglong::box::OCI::Runtime::Start called ");
    SPDLOG_TRACE("[containerID=\"{}\", interactive={}, consoleSocket=\"{}\", extraFDs={}, boxAsInit={}]", containerID,
                 interactive, consoleSocket, extraFDs, boxAsInit);
    // using linglong::util::FlockGuard;

    // auto containerWorkingDir = this->workingDir / containerID;

    // try {
    // std::unique_ptr<Container> c;

    // {
    // auto lock = FlockGuard(this->workingDir);
    // c.reset(new Container(containerWorkingDir));
    // }

    // std::vector<util::FD> fds;

    // for (int i = STDERR_FILENO + 1; i <= STDERR_FILENO + extraFDs; i++) {
    // fds.push_back(util::FD(i));
    // }

    // c->Start(consoleSocket, std::move(fds), consoleSocket, boxAsInit);

    // {
    // auto lock = FlockGuard(this->workingDir);
    // this->updateState(containerWorkingDir, c->state);
    // }

    // if (interactive) {
    // // TODO: check stdin/stdout is a tty
    // proxy(0, 1, c->socket.fd, c->terminalFD->fd);
    // }

    // } catch (...) {
    // std::throw_with_nested(std::runtime_error("Command start failed"));
    // }
}

void Runtime::Kill(const std::string &containerID, const int &signal)
{
    SPDLOG_TRACE("linglong::box::OCI::Runtime::Kill called");
    SPDLOG_TRACE("[containerID=\"{}\", signal={}]", containerID, signal);
    // using linglong::util::FlockGuard;

    // auto containerWorkingDir = this->workingDir / containerID;

    // try {
    // std::unique_ptr<Container> c;

    // {
    // auto lock = FlockGuard(this->workingDir);
    // c.reset(new Container(containerWorkingDir));
    // }

    // c->Kill(sig);

    // } catch (...) {
    // std::throw_with_nested(std::runtime_error("Command kill failed"));
    // }
}

void Runtime::Delete(const std::string &containerID)
{
    SPDLOG_TRACE("linglong::box::OCI::Runtime::Kill called");
    SPDLOG_TRACE("[containerID=\"{}\"]", containerID);

    // using linglong::util::FlockGuard;

    // auto containerWorkingDir = this->workingDir / containerID;

    // try {
    // std::unique_ptr<Container> c;

    // {
    // auto lock = FlockGuard(this->workingDir);
    // c.reset(new Container(containerWorkingDir));
    // }

    // c->Delete();

    // {
    // auto lock = FlockGuard(this->workingDir);
    // std::filesystem::remove_all(this->workingDir);
    // }

    // } catch (...) {
    // std::throw_with_nested(std::runtime_error("Command kill failed"));
    // }
}

std::string Runtime::State(const std::string &containerID)
{
    SPDLOG_TRACE("linglong::box::OCI::Runtime::State called");
    SPDLOG_TRACE("[containerID=\"{}\"]", containerID);
    // using linglong::util::FlockGuard;

    // auto containerWorkingDir = this->workingDir / containerID;

    // try {
    // std::unique_ptr<Container> c;

    // {
    // auto lock = FlockGuard(this->workingDir);
    // c.reset(new Container(containerWorkingDir));
    // }

    // nlohmann::json ret;
    // ret = c->state;
    // // return ret;

    // } catch (...) {
    // std::throw_with_nested(std::runtime_error("Command state failed"));
    // }
    return "TODO";
}

std::vector<std::string> Runtime::List() const
{
    SPDLOG_TRACE("linglong::box::OCI::Runtime::List called");
    // using linglong::util::FlockGuard;

    // try {
    // auto lock = FlockGuard(this->workingDir);
    // std::vector<std::string> ret;
    // for (const auto &entry : std::filesystem::directory_iterator(this->workingDir)) {
    // ret.push_back(entry.path().filename());
    // }
    // return ret;
    // } catch (...) {
    // std::throw_with_nested(std::runtime_error("Command list failed"));
    // }
    return {};
}

void Runtime::Exec(const std::string &containerID, util::FD processJsonPath, const bool interactive,
                   const std::string &consoleSocket, const int extraFDs)
{
    SPDLOG_TRACE("linglong::box::OCI::Runtime::Exec called");
    SPDLOG_TRACE("[containerID=\"{}\", processJsonPath=\"{}\", interactive={}, consoleSocket=\"{}\", extraFDs={}]",
                 containerID, processJsonPath.path(), interactive, consoleSocket, extraFDs);

    // using linglong::util::FlockGuard;
    // auto containerWorkingDir = this->workingDir / containerID;

    // try {
    // std::unique_ptr<Container> c;
    // {
    // auto lock = FlockGuard(this->workingDir);
    // c.reset(new Container(containerWorkingDir));
    // }
    // std::ifstream processJsonFile;
    // processJsonFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    // processJsonFile.open(pathToProcess);

    // nlohmann::json processJson;
    // processJsonFile >> processJson;

    // auto fds = std::vector<std::shared_ptr<util::FD>> {
    // std::make_shared<util::FD>(dup(STDIN_FILENO)),
    // std::make_shared<util::FD>(dup(STDOUT_FILENO)),
    // std::make_shared<util::FD>(dup(STDERR_FILENO)),
    // };

    // util::Message m({{{"command", "exec"}, {"process", processJson}}, fds});

    // } catch (...) {
    // std::throw_with_nested(std::runtime_error("Command list failed"));
    // }
}

void Runtime::Exec(const std::string &containerID, const std::vector<std::string> &commandToExec,
                   const bool interactive, const std::string &consoleSocket, const int extraFDs)
{
    SPDLOG_TRACE("linglong::box::OCI::Runtime::Exec called");
    SPDLOG_TRACE("[containerID=\"{}\", commandToExec=\"{}\", interactive={}, consoleSocket=\"{}\", extraFDs={}]",
                 containerID, util::strjoin(commandToExec), interactive, consoleSocket, extraFDs);
}

// static void copy(int in, int out)
// {
// while (true) {
// int nread = copy_file_range(in, nullptr, out, nullptr, 4096, 0);
// if (nread < 0) {
// if (errno == EIO)
// return;
// throw std::runtime_error(fmt::format("Failed to copy_file_range: {}", strerror(errno)));
// }
// continue;
// }
// }

// void Runtime::proxy(int in, int out, int notify, int target)
// {
// util::Epoll epoll;
// epoll.add(in, [in, target](util::Epoll &epoll, const epoll_event &event) { copy(in, target); });
// epoll.add(target, [out, target](util::Epoll &epoll, const epoll_event &event) { copy(target, out); });
// epoll.add(notify, [](util::Epoll &epoll, const epoll_event &event) { epoll.end(); });
// epoll.run();
// }

} // namespace linglong::box::OCI
