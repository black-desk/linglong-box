#include <sys/socket.h>

#include <sstream>

#include <spdlog/spdlog.h>

#include "init.h"
#include "util/epoll.h"
#include "util/exception.h"
#include "util/sync.h"

void init(int listenFD, int startFD, int appPID)
{
    try {
        int ret = listen(listenFD, 1);
        if (ret) {
            throw std::runtime_error(fmt::format("Failed to call listen: {}", std::strerror(errno)));
        }

        Epoll epoll;

        auto handleConn = [](Epoll &epoll, const epoll_event &event) {
            int connFD = event.data.fd;
            linglong::util::Pipe a(connFD);
            a>>;
        };

        epoll.add(listenFD, [handleConn, listenFD](Epoll &epoll, const epoll_event &event) {
            int connFD = accept(listenFD, nullptr, nullptr);
            epoll.add(connFD, handleConn);
        });

    } catch (const std::exception &e) {
        std::stringstream buf;
        linglong::util::printException(buf, e);
        spdlog::error("init failed: {}", buf);
    }
}
