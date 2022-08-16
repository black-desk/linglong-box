#ifndef LINGLONG_BOX_SRC_UTIL_SOCKET
#define LINGLONG_BOX_SRC_UTIL_SOCKET

#include "nlohmann/json.hpp"
#include <utility>
#include <sstream>
#include <vector>

namespace linglong::util {

struct Socket {
    Socket(int fd);
    ~Socket();

    void operator<<(const nlohmann::json &j);
    void operator>>(nlohmann::json &j);

    void operator<<(int i);
    void operator>>(int &i);
};

std::pair<int,int> SocketPair();
} // namespace linglong::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_SOCKET */
