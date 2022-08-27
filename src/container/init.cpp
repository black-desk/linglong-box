#include <spdlog/spdlog.h>

#include <sys/prctl.h>

#include "container.h"
#include "util/exception.h"
#include "util/sync.h"

namespace linglong {

Container::Init::Init(Container *const container)
    : container(container)
    , cloneFlag()
{
    // TODO:
}

void Container::Init::init(pid_t ppid) noexcept
{
    int ret = -1;

    // TODO: setup signal handlers

    makeSureParentSurvive(ppid);

    ret = prctl(PR_SET_CHILD_SUBREAPER, 1);
    if (ret) {
        spdlog::warn("Failed to set CHILD_SUBREAPER: {}", strerror(errno));
    }
}

void Container::Init::run()
{
    int ppid = getppid();

    std::function<int()> lambda = [this, ppid]() -> int {
        spdlog::debug("init: start");
        this->init(ppid);

        try {
            container->rootfs->sync << 0; // request rootfs to write ID mapping

            int msg = -1;
            spdlog::debug("init: waiting rootfs to write ID mapping");
            this->sync >> msg;
            spdlog::debug("init: done");
            if (msg == -1) {
                throw util::RuntimeError("Error during waiting monitor to write ID mapping");
            }

            this->setupContainer();

            this->container->sync << getpid(); // FIXME:


        } catch (const util::RuntimeError &e) {
            std::stringstream s;
            util::printException(s, e);
            spdlog::error("init: Unhanded exception during init running: {}", s);
        } catch (const std::exception &e) {
            spdlog::error("init: Unhanded exception during init running: {}", e.what());
        } catch (...) {
            spdlog::error("init: Unhanded exception during init running");
        }

        container->sync << -1;
        container->monitor->sync << -1;
        container->rootfs->sync << -1;
        return -1;
    };

    int initPID = clone(_, this->stackTop, this->cloneFlag, &lambda);
    if (initPID) { // parent
        this->pid = initPID;
        return;
    }
}
} // namespace linglong
