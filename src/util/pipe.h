#ifndef LINGLONG_BOX_SRC_UTIL_PIPE_H_
#define LINGLONG_BOX_SRC_UTIL_PIPE_H_

#include "fd.h"

namespace linglong::box::util {

inline void readWithRetry(const int fd, const void *const buf,
                          const std::size_t &nbytes)
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
    } while (len && ret > 0
             || (ret == -1 && (errno == EAGAIN || errno == EINTR)));

    if (ret == -1) {
        auto err = fmt::system_error(errno, "failed to read from fd={}", fd);
        SPDLOG_ERROR(err.what());
        throw err;
    }

    if (ret == 0 && len) {
        auto err =
            std::runtime_error(fmt::format("unexpected EOF from fd={}", fd));
        SPDLOG_ERROR(err.what());
        throw err;
    }

    return;
}

inline void writeWithRetry(const int fd, const void *const buf,
                           const std::size_t &nbytes)
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
    } while (len && ret >= 0
             || (ret == -1 && (errno == EAGAIN || errno == EINTR)));

    if (ret == -1) {
        auto err = fmt::system_error(errno, "failed to write to fd={}", fd);
        SPDLOG_ERROR(err.what());
        throw err;
    }

    return;
}

struct ReadableFD : public FD {
    ReadableFD(FD &&that)
        : FD(std::move(that))
    {
    }

    ReadableFD(int fd)
        : FD(fd)
    {
    }

    ReadableFD &operator>>(int &x)
    {
        SPDLOG_TRACE("read int from fd={}", this->__fd);
        readWithRetry(this->__fd, &x, sizeof(int));
        SPDLOG_TRACE("done, get int={}", x);
        return *this;
    }

    ReadableFD &operator>>(std::string &str)
    {
        SPDLOG_TRACE("read string from fd={}", this->__fd);
        int len;
        *this >> len;
        str.resize(len);
        readWithRetry(this->__fd, &str[0], len);
        return *this;
    }

    void reset()
    {
        if (::lseek(this->__fd, 0, SEEK_SET)) {
            auto err =
                fmt::system_error(errno, "failed to reset fd={} read offset");
            SPDLOG_ERROR(err);
            throw err;
        }
    }
};

struct WriteableFD : public FD {
    WriteableFD(FD &&that)
        : FD(std::move(that))
    {
    }

    WriteableFD(int fd)
        : FD(fd)
    {
    }

    WriteableFD &operator<<(const int &x)
    {
        SPDLOG_TRACE("write int={} to fd={}", x, this->__fd);
        writeWithRetry(this->__fd, &x, sizeof(int));
        SPDLOG_TRACE("done");
        return *this;
    }

    WriteableFD &operator<<(const std::string &str)
    {
        SPDLOG_TRACE("write string={} to fd={}", str, this->__fd);
        auto len = str.length();
        *this << len;
        writeWithRetry(this->__fd, str.c_str(), len);
        SPDLOG_TRACE("done");
        return *this;
    }
};

inline std::tuple<ReadableFD, WriteableFD> pipe()
{
    int ends[2];
    if (::pipe2(ends, O_CLOEXEC)) {
        auto err = fmt::system_error(errno, "failed to create pipe");
        SPDLOG_ERROR(err.what());
        throw err;
    }
    return {ReadableFD(ends[0]), WriteableFD(ends[1])};
}

} // namespace linglong::box::util

#endif /* LINGLONG_BOX_SRC_UTIL_PIPE_H_ */
