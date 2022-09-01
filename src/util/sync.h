#ifndef LINGLONG_BOX_SRC_UTIL_SYNC_H
#define LINGLONG_BOX_SRC_UTIL_SYNC_H

#include "util/fd.h"
#include <utility>
#include <sstream>
#include <vector>

#include <nlohmann/json.hpp>

namespace linglong::util {
struct Message {
    nlohmann::json raw;
    std::vector<util::FD> fds;
};

class Pipe
{
public:
    Pipe();
    Pipe(std::unique_ptr<util::FD> socket); // fd is a local socket.
    Pipe &operator<<(int);
    Pipe &operator>>(int &);
    Pipe &operator<<(Message);
    Pipe &operator>>(Message &);
};

} // namespace linglong::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_SOCKET */
