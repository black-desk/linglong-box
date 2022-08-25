#ifndef LINGLONG_BOX_SRC_UTIL_SYNC_H
#define LINGLONG_BOX_SRC_UTIL_SYNC_H

#include "nlohmann/json.hpp"
#include <utility>
#include <sstream>
#include <vector>

namespace linglong::util {
class Pipe{
    public:
    Pipe& operator<<(int);
    Pipe& operator>>(int&);
};
} // namespace linglong::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_SOCKET */
