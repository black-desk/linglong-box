#include <fcntl.h>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <stdexcept>
#include <unistd.h>

#include "nlohmann/json.hpp"
#include "runtime.h"

#include "spdlog/spdlog.h"
#include "util/exception.h"
#include "util/filesystem.h"
#include "util/lock.h"
#include "container/container.h"

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
        if (!mkdirp(this->workingDir, 0755)) {
            auto msg = fmt::format("Failed to create dir (path=\"{}\")", this->workingDir);
            throw std::runtime_error(msg.c_str());
        }

        {
            auto guard = FlockGuard(this->workingDir);

            if (!mkdir(containerWorkingDir, 0755)) {
                auto msg = fmt::format("Failed to create dir (path=\"{}\")", containerWorkingDir);
                throw std::runtime_error(msg.c_str());
            }

            auto originConfigJsonPath = bundle / std::filesystem::path("config.json");
            std::ifstream configJsonFile(originConfigJsonPath);
            if (!configJsonFile.is_open()) {
                auto msg = fmt::format("Failed to open config.json (\"{}\")", originConfigJsonPath);
                throw std::runtime_error(msg.c_str());
            }

            nlohmann::json configJson;
            configJsonFile >> configJson;

            container.reset(new Container(containerID, bundle, configJson, containerWorkingDir, 1));

            this->updateState(containerWorkingDir, container->state);
        }

        container->Create();

        {
            int msg = 0;
            spdlog::debug("runtime: wait init to send create result");
            container->sync >> msg;
            spdlog::debug("runtime: done");
            if (msg == -1) {
                throw util::RuntimeError("Failed to create container");
            } else {
                container->state.pid = msg;
            }
        }

        {
            auto guard = FlockGuard(this->workingDir);
            this->updateState(containerWorkingDir, container->state);
        }

        container->monitor->sync << 0; // request run "createRuntime"

        {
            int msg = 0;
            spdlog::debug("runtime: wait monitor to report hooks result");
            container->sync >> msg;
            spdlog::debug("runtime: done");
            if (msg != 0) {
                throw util::RuntimeError("Error during waitting hooks finish");
            }
        }
    } catch (const std::runtime_error &e) {
        auto guard = FlockGuard(this->workingDir);
        rmrf(containerWorkingDir);
        auto msg = fmt::format("Failed to create container (name=\"{}\", bundle=\"{}\")", containerID, bundle);
        std::throw_with_nested(std::runtime_error(msg.c_str()));
    }
}

void Runtime::updateState(const std::filesystem::path &containerWorkingDir, const struct State &state)
{
    auto stateJsonPath = containerWorkingDir / std::filesystem::path("state.json");
    std::ofstream stateJsonFile(stateJsonPath);
    if (!stateJsonFile.is_open()) {
        throw util::RuntimeError(fmt::format("Failed to open file (\"{}\")", stateJsonPath));
    }

    nlohmann::json stateJson(state);
    stateJsonFile << stateJson;
}

void Runtime::Start(const std::string &containerID, const bool interactive)
{
    // FIXME: TODO
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

} // namespace OCI
} // namespace linglong
