#ifndef LINGLONG_BOX_SRC_UTIL_EPOLL_H
#define LINGLONG_BOX_SRC_UTIL_EPOLL_H

#include <functional>
#include <sys/epoll.h>

#include <fmt/format.h>

#include "util/common.h"
#include "util/fd.h"

namespace linglong::util {

inline void epoll_ctl_add(int epfd, int fd)
{
    static epoll_event ev = {};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    if (ret != 0) {
        throw std::runtime_error(fmt::format("Failed to fd={} to epoll: {}", fd, strerror(errno)));
    }
}

class Epoll : public NonCopyable
{
public:
    Epoll();
    void add(int fd, std::function<void(Epoll &epoll, const epoll_event &event)>);
    void remove(int fd);
    void end();
    void run();
};

} // namespace linglong::util

#endif
