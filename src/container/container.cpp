

#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <wait.h>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "container.h"

#include "util/exception.h"
#include "util/wait.h"

namespace linglong {

static int doCreate(void *arg);

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

    auto init = std::unique_ptr<Init>(new Init(this->workingPath / "socket"));

    auto rootfs = std::unique_ptr<Rootfs>();
    if (this->config->annotations->rootfs.has_value()) {
        rootfs.reset(new Rootfs(this->config->annotations->rootfs.value()));
    }

    this->monitor.reset(new Monitor(std::move(init), std::move(rootfs), std::move(this->config)));
    this->monitor->create();
}

void Container::Monitor::create()
{
    
    auto rootfsPID = clone(doCreate, this->rootfs->stackTop, this->rootfs->cloneFlag, this);
    if (rootfsPID) { // parent
        int wstatus;
        if (waitpid(rootfsPID, &wstatus, 0) == -1) {
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
    }
}

static int doCreate(void *arg)
{
    return -1;
}

Container::Rootfs::Rootfs(const OCI::Config::Annotations::Rootfs &config)
    : cloneFlag(CLONE_VFORK | CLONE_NEWUSER | CLONE_NEWNS | SIGCHLD)
    , stackSize(1024 * 1024)
{
}

void Container::Rootfs::allocateStack()
{
    char *stack =
        (char *)mmap(NULL, this->stackSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        throw util::RuntimeError(
            fmt::format("mmap child stack for rootfs preparer failed: {} (errno={})", std::strerror(errno), errno));
    }
    this->stackTop = stack + stackSize; /* Assume stack grows downward */
}

} // namespace linglong
