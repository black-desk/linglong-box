// #include <csignal>
// #include <sys/socket.h>
// #include <sys/signalfd.h>

// #include <sstream>

// #include <spdlog/spdlog.h>
// #include <sys/wait.h>

// #include "init.h"
// #include "oci/config.h"
// #include "oci/util.h"
// #include "util/epoll.h"
// #include "util/exception.h"
// #include "util/fd.h"

namespace linglong::box {
// int init(int listenFD, int startFD, int appPID)
int init(int argc, char **argv)
{
    // try {
    // linglong::util::FD _(listenFD);
    // int ret = listen(listenFD, 1);
    // if (ret) {
    // throw std::runtime_error(fmt::format("Failed to call listen: {}", std::strerror(errno)));
    // }

    // linglong::util::Epoll epoll;
    // std::map<int, std::shared_ptr<linglong::util::Socket>> fdmap;
    // fdmap.insert(std::make_pair(appPID, new linglong::util::Socket(startFD)));

    // auto handleConn = [&fdmap](linglong::util::Epoll &epoll, const epoll_event &event) {
    // linglong::util::Message msg;
    // auto conn = std::shared_ptr<linglong::util::Socket>(new linglong::util::Socket(event.data.fd));
    // *conn >> msg;
    // if (msg.raw.at("command") == "exec") {
    // linglong::OCI::Config::Process p;
    // msg.raw.at("process").get_to(p);
    // p.parse();
    // pid_t app = linglong::OCI::execProcess(p);
    // fdmap.insert(std::make_pair(app, conn));
    // } else {
    // spdlog::warn("unexcepted messge: \"{}\"", to_string(msg.raw));
    // }
    // };

    // epoll.add(listenFD, [&handleConn](linglong::util::Epoll &epoll, const epoll_event &event) {
    // int connFD = accept(event.data.fd, nullptr, nullptr);
    // epoll.add(connFD, handleConn);
    // });

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

    // ret = sigprocmask(SIG_BLOCK, &mask, NULL);
    // if (ret == -1) {
    // throw fmt::system_error(errno, "Failed to call sigprocmask to block signal");
    // }

    // auto sfd = linglong::util::FD(ret);
    // epoll.add(ret, [&fdmap](linglong::util::Epoll &epoll, const epoll_event &event) {
    // signalfd_siginfo info;
    // int ret = read(event.data.fd, &info, sizeof(info));
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
    // auto it = fdmap.find(pid);
    // if (it != fdmap.end()) {
    // *it->second << linglong::util::Message({{"messgae", "end", "stat", stat}, {}});
    // fdmap.erase(it);
    // }
    // }
    // } else if (info.ssi_signo == SIGTERM) {
    // epoll.end();
    // }
    // });
    // return 0;
    // } catch (const std::exception &e) {
    // std::stringstream buf;
    // linglong::util::printException(buf, e);
    // spdlog::error("init failed: {}", buf);
    // } catch (...) {
    // spdlog::error("init failed");
    // }
    // return -1;
}
} // namespace linglong::box
