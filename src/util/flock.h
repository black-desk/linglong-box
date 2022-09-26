#ifndef LINGLONG_BOX_SRC_UTIL_FLOCK_H_
#define LINGLONG_BOX_SRC_UTIL_FLOCK_H_

#include <sys/file.h>

#include "util/common.h"
#include "util/fd.h"

namespace linglong::box::util {

struct FLockGuard : public NonCopyable {
    FD fd;

    FLockGuard(FD &dirFD, bool share = true, bool block = true)
        : fd(dirFD.at("lock", O_CREAT))
    {
        SPDLOG_TRACE("request flock on {}", fd.path());

        auto operation = (share ? LOCK_SH : LOCK_EX) | (block ? 0 : LOCK_NB);
        SPDLOG_TRACE("[fd={}, share={}, block={}, operation={}]", fd.__fd, share, block, operation);

        if (::flock(fd.__fd, operation)) {
            auto err = fmt::system_error(errno, "flock failed (fd={}, operation={})", fd.__fd, operation);
            spdlog::error(err.what());
            throw err;
        }
    }

    FLockGuard(FLockGuard &&that)
        : fd(std::move(that.fd))
    {
    }

    FLockGuard &operator=(FLockGuard &&that)
    {
        this->fd = std::move(that.fd);
        return *this;
    }

    ~FLockGuard()
    {
        SPDLOG_TRACE("request funlock on {}, fd={}", fd.path(), fd.__fd);

        if (::flock(fd.__fd, LOCK_UN)) {
            spdlog::critical("flock unlock failed (errno={}): {}", errno, std::strerror(errno));
        }
    }
};

} // namespace linglong::box::util

#endif
