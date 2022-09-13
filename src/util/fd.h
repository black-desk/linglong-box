#ifndef LINGLONG_BOX_SRC_UTIL_FD_H
#define LINGLONG_BOX_SRC_UTIL_FD_H
#include <unistd.h>

#include <nlohmann/json.hpp>

#include "common.h"

namespace linglong::util {

struct FD
    : public NonCopyable
    , public NonMoveable {
    int fd;

    FD(int);
    ~FD();
};

struct PipeReadEnd : public FD {
    PipeReadEnd(int);
    PipeReadEnd &operator>>(int &x);
};

struct PipeWriteEnd : public FD {
    PipeWriteEnd(int);
    PipeWriteEnd &operator<<(const int &x);
};

struct Pipe {
    Pipe();
    std::unique_ptr<PipeReadEnd> read;
    std::unique_ptr<PipeWriteEnd> write;
    PipeReadEnd &operator>>(int &x);
    PipeWriteEnd &operator<<(const int &x);
};

std::pair<std::shared_ptr<PipeReadEnd>, std::shared_ptr<PipeWriteEnd>> pipe();

struct Message {
    nlohmann::json raw;
    std::vector<std::shared_ptr<util::FD>> fds;
};

struct Socket : public FD {
    Socket(int);
    Socket &operator>>(Message &x);
    Socket &operator<<(const Message &x);
};

} // namespace linglong::util

#endif /* ifndef SYMBOL */
