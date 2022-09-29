// #include "monitor.h"
// #include "oci/config.h"
// #include "oci/util.h"
// #include "spdlog/spdlog.h"
// #include "util/exception.h"

// #include <fmt/format.h>

// #include <sys/signalfd.h>
// #include <sys/signal.h>
// #include <sys/wait.h>
// #include <unistd.h>

// #include <util/fd.h>

#include <memory>

#include "docopt.cpp/docopt.h"
#include "nlohmann/json.hpp"

#include "lib/container/monitor.h"
#include "util/args.h"
#include "util/common.h"
#include "util/exec.h"
#include "util/log.h"
#include "util/exception.h"

namespace linglong::box {

using std::string;
using nlohmann::json;

// void monitor(int initPID, int rootfsPID, std::string rawJson)

static const char USAGE[] = R"(Usage:
  ll-box-monitor --config=<config_fd>
                 --socket=<socket_fd>
                 --runtime_write=<runtime_write_fd>
                 --monitor_read=<monitor_read_fd>
                 --monitor_write=<monitor_write_fd>
                 --rootfs_read=<rootfs_read_fd>
                 --rootfs_write=<rootfs_write_fd>
                 --init_read=<init_read_fd>
                 --init_write=<init_write_fd>
)";

int monitor(int argc, char **argv) noexcept
{
    util::init_logger("ll-box-monitor");

    SPDLOG_DEBUG("ll-box-monitor started");

    auto args = util::parseArgs(USAGE, argc, argv, false);

    try {
        std::unique_ptr<container::Monitor> m;
        {
            auto configFD =
                util::ReadableFD(args.find("--config")->second.asLong());
            auto socket = util::FD(args.find("--socket")->second.asLong());
            auto runtimeWrite =
                util::FD(args.find("--runtime_write")->second.asLong());
            auto rootfsRead =
                util::FD(args.find("--rootfs_read")->second.asLong());
            auto rootfsWrite =
                util::FD(args.find("--rootfs_write")->second.asLong());
            auto monitorRead =
                util::FD(args.find("--monitor_read")->second.asLong());
            auto monitorWrite =
                util::FD(args.find("--monitor_write")->second.asLong());
            auto initRead = util::FD(args.find("--init_read")->second.asLong());
            auto initWrite =
                util::FD(args.find("--init_write")->second.asLong());

            {
                configFD.reset();
                std::string configStr;
                configFD >> configStr;
                m.reset(new container::Monitor(
                    json::parse(configStr).get<OCI::Config>(),
                    runtimeWrite.dup(), rootfsWrite.dup(),
                    std::move(monitorRead), initWrite.dup()));
            }

            m->startRootfsPrepaer(std::move(configFD), std::move(socket),
                                  std::move(rootfsRead), std::move(rootfsWrite),
                                  std::move(monitorWrite), std::move(initWrite),
                                  std::move(initRead));
        }

        m->handleHooks();

        return 0;

    } catch (const std::exception &e) {
        std::cerr << fmt::format("ll-box-monitor: failed:\n{}",
                                 util::nestWhat(e));
    } catch (...) {
        std::cerr << fmt::format("ll-box-monitor: failed");
        return -1;
    }

    // sigset_t mask;
    // if (sigemptyset(&mask)) {
    // throw fmt::system_error(errno, "Failed to call sigemptyset");
    // }
    // if (sigaddset(&mask, SIGCHLD)) {
    // throw fmt::system_error(errno, "Failed to call sigaddset SIGCHLD");
    // }
    // if (sigaddset(&mask, SIGTERM)) {
    // throw fmt::system_error(errno, "Failed to call sigaddset SIGTERM");
    // }

    // int ret = sigprocmask(SIG_BLOCK, &mask, NULL);
    // if (ret == -1) {
    // throw fmt::system_error(errno, "Failed to call sigprocmask to block
    // signal");
    // }

    // auto sfd = linglong::util::FD(ret);

    // do {
    // signalfd_siginfo info;
    // ret = read(sfd.fd, &info, sizeof(info));
    // if (ret != sizeof(info)) {
    // throw fmt::system_error(errno, "Failed to get signalfd_siginfo");
    // }
    // if (info.ssi_signo == SIGCHLD) {
    // while (true) {
    // int stat;
    // int pid = waitpid(-1, &stat, WNOHANG);
    // if (pid == -1) {
    // throw fmt::system_error(errno, "Failed to wait dead child process");
    // }
    // if (pid == 0) {
    // break;
    // }

    // if (WIFEXITED(stat)) {
    // if (WEXITSTATUS(stat) != 0) {
    // spdlog::warn("child (pid={}) exited, status=%d", pid, WEXITSTATUS(stat));
    // } else {
    // spdlog::warn("child (pid={}) exited, status=%d", pid, WEXITSTATUS(stat));
    // }
    // } else if (WIFSIGNALED(stat)) {
    // spdlog::warn("child (pid={}) killed by signal {}", pid, WTERMSIG(stat));
    // } else if (WIFSTOPPED(stat)) {
    // spdlog::debug("child (pid={}) stopped by signal {}", pid,
    // WTERMSIG(stat)); } else if (WIFCONTINUED(stat)) { spdlog::debug("child
    // (pid={}) continued", pid);
    // }
    // }
    // } else if (info.ssi_signo == SIGTERM) {
    // break;
    // }
    // } while (ret != -1);

    // linglong::OCI::Config c;
    // auto j = nlohmann::json::parse(rawJson);
    // j.get_to(c);

    // if (c.hooks.has_value() && c.hooks->poststop.has_value()) {
    // for (const auto &hook : c.hooks->poststop.value()) {
    // try {
    // linglong::OCI::execHook(hook);
    // } catch (const std::exception &e) {
    // std::stringstream buf;
    // linglong::util::printException(buf, e);
    // spdlog::error("poststop hook {} failed: {}", hook.path, buf);
    // } catch (...) {
    // spdlog::error("poststop hook {} failed", hook.path);
    // }
    // }
    // }

    // ret = kill(-rootfsPID, SIGTERM);
    // if (ret == -1) {
    // throw fmt::system_error(errno, "failed to release rootfs");
    // }

    // return 0;
}

} // namespace linglong::box
