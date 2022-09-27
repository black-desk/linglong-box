#ifndef LINGLONG_BOX_SRC_UTIL_PIPE_H_
#define LINGLONG_BOX_SRC_UTIL_PIPE_H_

#include "fd.h"

namespace linglong::box::util {

inline void readWithRetry(const int fd, const void *const buf, const std::size_t &nbytes)
{
    int ret, start = 0, len = nbytes;
    do {
        assert(start < nbytes);
        assert(len > 0);
        ret = ::read(fd, (char *)buf + start, len);
        if (ret > 0) {
            start += ret;
            len -= ret;
        }
    } while (len && ret >= 0 || (ret == -1 && (errno == EAGAIN || errno == EINTR)));

    if (ret == -1) {
        throw fmt::system_error(errno, "failed to read from {}", fd);
    }

    return;
}

inline void writeWithRetry(const int fd, const void *const buf, const std::size_t &nbytes)
{
    int ret, start = 0, len = nbytes;
    do {
        assert(start < nbytes);
        assert(len > 0);
        ret = ::write(fd, (char *)buf + start, len);
        if (ret > 0) {
            start += ret;
            len -= ret;
        }
    } while (len && ret >= 0 || (ret == -1 && (errno == EAGAIN || errno == EINTR)));

    if (ret == -1) {
        throw fmt::system_error(errno, "failed to read from {}", fd);
    }

    return;
}

struct PipeReadEnd : private FD {
    PipeReadEnd(int fd)
        : FD(fd)
    {
    }

    PipeReadEnd &operator>>(int &x)
    {
        SPDLOG_TRACE("read int from fd={}", this->__fd);
        readWithRetry(this->__fd, &x, sizeof(int));
        SPDLOG_TRACE("done, get int={}", x);
        return *this;
    }
};

struct PipeWriteEnd : public FD {
    PipeWriteEnd(int fd)
        : FD(fd)
    {
    }

    PipeWriteEnd &operator<<(const int &x)
    {
        SPDLOG_TRACE("write int={} to fd={}", x, this->__fd);
        writeWithRetry(this->__fd, &x, sizeof(int));
        SPDLOG_TRACE("done");
        return *this;
    }
};

inline std::tuple<PipeReadEnd, PipeWriteEnd> pipe()
{
    int ends[2];
    if (::pipe(ends)) {
        auto err = fmt::system_error(errno, "failed to create pipe");
        SPDLOG_ERROR(err.what());
        throw err;
    }
    return {PipeReadEnd(ends[0]), PipeWriteEnd(ends[1])};
}

} // namespace linglong::box::util

#endif /* LINGLONG_BOX_SRC_UTIL_PIPE_H_ */