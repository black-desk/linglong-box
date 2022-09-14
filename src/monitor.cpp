#include "monitor.h"
#include "oci/config.h"
#include "oci/util.h"
#include "spdlog/spdlog.h"

#include <fmt/format.h>

#include <sys/signalfd.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <util/fd.h>

void monitor(int initPID, std::string rawJson)
{
    sigset_t mask;
    if (sigemptyset(&mask)) {
        throw fmt::system_error(errno, "Failed to call sigemptyset");
    }
    if (sigaddset(&mask, SIGCHLD)) {
        throw fmt::system_error(errno, "Failed to call sigaddset SIGCHLD");
    }
    if (sigaddset(&mask, SIGTERM)) {
        throw fmt::system_error(errno, "Failed to call sigaddset SIGTERM");
    }

    int ret = sigprocmask(SIG_BLOCK, &mask, NULL);
    if (ret == -1) {
        throw fmt::system_error(errno, "Failed to call sigprocmask to block signal");
    }

    auto sfd = linglong::util::FD(ret);

    do {
        signalfd_siginfo info;
        ret = read(sfd.fd, &info, sizeof(info));
        if (ret != sizeof(info)) {
            throw fmt::system_error(errno, "Failed to get signalfd_siginfo");
        }
        if (info.ssi_signo == SIGCHLD) {
            while (true) {
                int stat;
                int pid = waitpid(-1, &stat, WNOHANG);
                if (pid == -1) {
                    throw fmt::system_error(errno, "Failed to wait dead child process");
                }
                if (pid == 0) {
                    break;
                }

                if (WIFEXITED(stat)) {
                    if (WEXITSTATUS(stat) != 0) {
                        spdlog::warn("child (pid={}) exited, status=%d", pid, WEXITSTATUS(stat));
                    } else {
                        spdlog::warn("child (pid={}) exited, status=%d", pid, WEXITSTATUS(stat));
                    }
                } else if (WIFSIGNALED(stat)) {
                    spdlog::warn("child (pid={}) killed by signal {}", pid, WTERMSIG(stat));
                } else if (WIFSTOPPED(stat)) {
                    spdlog::debug("child (pid={}) stopped by signal {}", pid, WTERMSIG(stat));
                } else if (WIFCONTINUED(stat)) {
                    spdlog::debug("child (pid={}) continued", pid);
                }
            }
        } else if (info.ssi_signo == SIGTERM) {
            break;
        }
    } while (ret != -1);

    linglong::OCI::Config c;
    auto j = nlohmann::json::parse(rawJson);
    j.get_to(c);

    if (c.hooks.has_value() && c.hooks->poststop.has_value()) {
        for (const auto &hook : c.hooks->poststop.value()) {
            linglong::OCI::execHook(hook);
        }
    }
}
