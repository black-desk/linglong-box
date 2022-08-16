

#include "container.h"

#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>

namespace linglong {

Container::Container(const std::string &containerID, const std::filesystem::path &bundle,
                     const nlohmann::json &configJson, const std::filesystem::path &workingPath, const Option &option)
    : ID(containerID)
    , workingPath(workingPath)
    , bundlePath(bundle)
    , option(option)
{
    configJson.get_to(this->config);
    this->config.parse(bundlePath);
}

void Container::Create()
{
    int pid = vfork();
    if (pid) { // parent
        return;
    } else { // child
        auto server = std::unique_ptr<Main::LocalServer>(new Main::LocalServer(this->workingPath / "socket"));

        auto rootfs = std::unique_ptr<Main::Rootfs>();
        if (this->config.annotations->rootfs.has_value()) {
            rootfs.reset(new Main::Rootfs(this->config.annotations->rootfs.value()));
        }

        this->main.reset(new Main(std::move(server), std::move(rootfs)));
        exit(this->main->create());
    }
}

int Container::Main::create()
{
    prctl(PR_SET_CHILD_SUBREAPER, 1);
    
    int flags =  | SIGCHLD;
}

} // namespace linglong
