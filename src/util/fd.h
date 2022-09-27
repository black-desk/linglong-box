#ifndef LINGLONG_BOX_SRC_UTIL_FD_H
#define LINGLONG_BOX_SRC_UTIL_FD_H
#include <fcntl.h>
#include <filesystem>
#include <unistd.h>

#include <cerrno>

#include "fmt/fmt.h"
#include "spdlog/spdlog.h"
#include "util/fmt.h"
#include "nlohmann/json.hpp"

#include "common.h"

namespace linglong::box::util {

struct FD : public NonCopyable {
    int __fd;

    FD(const std::filesystem::path &path, int flag)
        : __fd([&path, &flag]() -> int {
            flag |= O_CLOEXEC;
            int fd = ::open(path.c_str(), flag);
            if (fd < 0) {
                auto err = fmt::system_error(errno, "failed to open \"{}\" with flags={}", path, flag);
                spdlog::error(err.what());
                throw err;
            }

            SPDLOG_TRACE("file \"{}\" opened with flags={}, fd={}", path, flag, fd);

            return fd;
        }())
    {
    }

    FD(int fd)
        : __fd(fd)
    {
        if (fd < 0) {
            auto err = fmt::system_error(EBADF, "fd invalided [errno={} msg=\"{}\"]", errno, std::strerror(errno));
            spdlog::error(err.what());
            throw err;
        }
    }

    FD(FD &&that)
        : __fd(that.__fd)
    {
        that.__fd = -1;
    }

    FD &operator=(FD &&that)
    {
        this->close();
        __fd = that.__fd;
        that.__fd = -1;
        return *this;
    }

    ~FD() { this->close(); }

    std::filesystem::path procPath() const { return fmt::format("/proc/self/fd/{}", this->__fd); }

    std::filesystem::path path() const
    {
        auto ret = std::filesystem::read_symlink(this->procPath());
        return std::move(ret);
    }

    FD at(const std::filesystem::path &path, int flag = O_PATH, int mode = S_IRUSR | S_IWUSR) const
    {
        flag |= O_CLOEXEC;
        auto ret = ::openat(this->__fd, path.c_str(), flag, mode);
        if (ret < 0) {
            auto err = fmt::system_error(errno, "failed to open \"{}\" with flag={} at \"{}\" (fd={})", path, flag,
                                         this->path(), this->__fd);
            SPDLOG_ERROR(err.what());
            throw err;
        }
        return FD(ret);
    }

    FD dup() const
    {
        auto ret = ::dup(this->__fd);
        if (ret < 0) {
            auto err = fmt::system_error(errno, "failed to dup fd={}", this->__fd);
            SPDLOG_ERROR(err.what());
            throw err;
        }
        return FD(ret);
    }

private:
    void close() noexcept
    {
        if (__fd != -1) {
            assert(__fd > STDERR_FILENO);

            try {
                SPDLOG_TRACE("file \"{}\" closed, fd={}", this->path(), this->__fd);
            } catch (...) {
            }

            ::close(__fd);
        }
    }
};

// struct Message {
// nlohmann::json raw;
// std::vector<std::shared_ptr<util::FD>> fds;
// };

// struct Socket : public FD {
// Socket(int);
// Socket &operator>>(Message &x);
// Socket &operator<<(const Message &x);
// };

} // namespace linglong::box::util

#endif /* ifndef SYMBOL */
