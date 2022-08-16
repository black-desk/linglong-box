

#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <wait.h>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "container.h"

#include "util/exception.h"
#include "util/wait.h"

namespace linglong {

Container::Container(const std::string &containerID, const std::filesystem::path &bundle,
                     const nlohmann::json &configJson, const std::filesystem::path &workingPath, const Option &option)
    : ID(containerID)
    , workingPath(workingPath)
    , bundlePath(bundle)
    , option(option)
    , isRef(false)
{
    this->config.reset(new OCI::Config);
    configJson.get_to(*this->config.get());
    this->config->parse(bundlePath);
}

void Container::Create()
{
    if (this->isRef) {
        throw util::RuntimeError(fmt::format("Cannot call create on a container reference."));
    }

    int pid = vfork();
    if (pid) { // parent
        int wstatus;
        if (waitpid(pid, &wstatus, 0) == -1) {
            spdlog::error("waitpid error: {} (errno={})", std::strerror(errno), errno);
            throw util::RuntimeError(fmt::format("cannot get create result"));
        } else {
            auto result = util::parse_wstatus(wstatus);
            if (result.first.has_value() && result.first.value() == 0) {
                return;
            } else {
                throw util::RuntimeError(fmt::format("Failed to create container {}", this->ID));
            }
        }
        return;
    } else { // child
        try {
            auto server = std::unique_ptr<Main::LocalServer>(new Main::LocalServer(this->workingPath / "socket"));

            auto rootfs = std::unique_ptr<Main::Rootfs>();
            if (this->config->annotations->rootfs.has_value()) {
                rootfs.reset(new Main::Rootfs(this->config->annotations->rootfs.value()));
            }

            this->main.reset(new Main(std::move(server), std::move(rootfs), std::move(this->config)));
            this->main->create();
        } catch (const std::exception &e) {
            spdlog::warn("Failed to create container: {}", e.what());
            exit(-1);
        }
        exit(0);
    }
}

void Container::Main::create()
{
    prctl(PR_SET_CHILD_SUBREAPER, 1);

    int flags = SIGCHLD;
}

} // namespace linglong
