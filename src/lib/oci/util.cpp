#include "util.h"

#include <iostream>
#include <fstream>
#include <ext/stdio_filebuf.h>

#include <fmt/format.h>

#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include "util/fd.h"
#include "util/wait.h"

namespace linglong::OCI {

pid_t execProcess(const OCI::Config::Process &process)
{
    util::Pipe sync;
    int appPID = fork();
    if (appPID) { // parent
        sync << 0;
        int ret;
        sync >> ret;
        if (ret) {
            throw std::runtime_error(
                fmt::format("execve \"{}\" failed: {}", nlohmann::json(process).dump(), strerror(errno)));
        }
        return appPID;
    } else {
        int ret;
        sync >> ret;

        std::unique_ptr<util::FD> pty(new util::FD(open("/dev/ptmx", O_RDWR | O_NOCTTY | O_CLOEXEC)));

        const uint PTSNAME_LEN = 64;
        char buf[PTSNAME_LEN];
        ret = ptsname_r(pty->fd, buf, sizeof(buf));
        if (ret < 0)
            throw std::runtime_error(fmt::format("Failed to get ptsname: {}", strerror(errno)));

        ret = unlockpt(pty->fd);
        if (ret < 0)
            throw std::runtime_error(fmt::format("Failed to unlockpt: {}", strerror(errno)));

        auto size = process.consoleSize;

        struct winsize ws;
        if (!size.has_value() && isatty(STDIN_FILENO)) {
            ret = ioctl(pty->fd, TIOCGWINSZ, &ws);
        if (ret < 0)
            spdlog::warn("Failed to get console size of stdin: {}", strerror(errno));
        }

        ws = {size->height, size->width};

        ret = ioctl(pty->fd, TIOCSWINSZ, &ws);
        if (ret < 0)
            throw std::runtime_error(fmt::format("Failed to set console size (height={}, width={}): {}", size->height,
                                                 size->width, strerror(errno)));
        int fdlimit = (int)sysconf(_SC_OPEN_MAX);
        for (int i = STDERR_FILENO + 1; i < fdlimit; i++)
            close(i);

        const auto &processArgs = process.args;
        const auto &processEnv = process.env.value_or(std::vector<std::string>());
        const char *args[processArgs.size() + 1];
        const char *env[processEnv.size() + 1];

        for (int i = 0; i < processArgs.size(); i++) {
            args[i] = processArgs[i].c_str();
        }
        args[processArgs.size()] = nullptr;

        for (int i = 0; i < processEnv.size(); i++) {
            env[i] = processEnv[i].c_str();
        }
        env[processEnv.size()] = nullptr;

        ret = execve(args[0], const_cast<char *const *>(args), const_cast<char *const *>(env));
        sync << errno;
        exit(-1);
    }
}

struct signalBlocker {
    signalBlocker(int signo)
    {
        sigemptyset(&mask);
        sigaddset(&mask, signo);
        int ret = sigprocmask(SIG_BLOCK, &mask, &oldmask);
        if (ret) {
            throw std::runtime_error(fmt::format("Failed to block signal: {}", strerror(errno)));
        }
    }
    ~signalBlocker()
    {
        int ret = sigprocmask(SIG_BLOCK, &oldmask, nullptr);
        if (ret) {
            spdlog::error("Failed to unblock signal: {}", strerror(errno));
        }
    }
    sigset_t mask, oldmask;
};

void execHook(const linglong::OCI::Config::Hooks::Hook &hook)
{
    try {
        int hookPipe[2];
        int ret = pipe(hookPipe);
        if (ret) {
            throw std::runtime_error(fmt::format("Failed to create pipe: {}", strerror(errno)));
        }

        // for clean up
        auto fd1 = util::FD(hookPipe[0]);
        auto fd2 = util::FD(hookPipe[1]);

        bool has_timeout = hook.timeout.has_value() && hook.timeout;
        time_t start = time(NULL);

        std::unique_ptr<signalBlocker> blocker;
        if (has_timeout) {
            blocker.reset(new signalBlocker(SIGCHLD));
        }

        int hookPID = fork();
        if (hookPID) { // father
            close(hookPipe[0]);
            if (has_timeout) {
                auto timeout = hook.timeout.value();
                for (time_t now = start; now - start < timeout; now = time(nullptr)) {
                    siginfo_t info;
                    int elapsed = now - start;
                    struct timespec ts_timeout = {.tv_sec = timeout - elapsed, .tv_nsec = 0};

                    ret = sigtimedwait(&blocker->mask, &info, &ts_timeout);
                    if (ret < 0 && errno != EAGAIN)
                        throw std::runtime_error(fmt::format("Failed to call sigtimedwait: {}", strerror(errno)));

                    if (info.si_signo == SIGCHLD && info.si_pid == hookPID)
                        break;

                    if (ret < 0 && errno == EAGAIN) {
                        ret = kill(hookPID, SIGKILL);
                        if (ret) {
                            throw std::runtime_error(fmt::format("Failed to kill timeout hook: {}", strerror(errno)));
                        }
                        throw std::runtime_error(fmt::format("hook \"{}\" timeout", hook.path));
                    }
                }
            }

            int wstatus;
            ret = waitpid(hookPID, &wstatus, 0);
            if (ret) {
                throw std::runtime_error(fmt::format("Failed to call waitpid: {}", strerror(errno)));
            }

            auto hookOutput = std::string(
                std::istreambuf_iterator<char>(new __gnu_cxx::stdio_filebuf<char>(hookPipe[1], std::ios::in)),
                std::istreambuf_iterator<char>());

            auto [termed, code] = util::parse_wstatus(wstatus);

            if (!termed && code != 0) {
                throw std::runtime_error(
                    fmt::format("hook failed, exit with {}, stdout & stderr = \"{}\"", code, hookOutput));
            }
        } else { // child
            ret = dup2(hookPipe[0], STDOUT_FILENO);
            if (ret) {
                throw std::runtime_error(fmt::format("Failed to dup pipe to STDOUT of hook: {}", strerror(errno)));
            }
            ret = dup2(hookPipe[0], STDERR_FILENO);
            if (ret) {
                throw std::runtime_error(fmt::format("Failed to dup pipe to STDERR of hook: {}", strerror(errno)));
            }

            int fdlimit = (int)sysconf(_SC_OPEN_MAX);
            for (int i = STDERR_FILENO + 1; i < fdlimit; i++)
                close(i);

            const auto &hookArgs = hook.args.value_or(std::vector<std::string>());
            const auto &hookEnv = hook.env.value_or(std::vector<std::string>());
            const char *args[hookArgs.size() + 1];
            const char *env[hookEnv.size() + 1];

            for (int i = 0; i < hookArgs.size(); i++) {
                args[i] = hookArgs[i].c_str();
            }
            args[hookArgs.size()] = nullptr;

            for (int i = 0; i < hookEnv.size(); i++) {
                env[i] = hookEnv[i].c_str();
            }
            env[hookEnv.size()] = nullptr;

            ret = execve(hook.path.c_str(), const_cast<char *const *>(args), const_cast<char *const *>(env));
            if (ret) {
                std::cerr << "execve \"" << hook.path << "\" failed with errno==" << errno << ":" << strerror(errno)
                          << std::endl;
                exit(-1);
            }
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error(fmt::format("execute hook {} failed", hook.path)));
    }
}

} // namespace linglong::OCI
