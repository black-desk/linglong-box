#ifndef LINGLONG_BOX_SRC_UTIL_LOCK_H
#define LINGLONG_BOX_SRC_UTIL_LOCK_H
#include <filesystem>
#include <istream>
namespace linglong::util {
class FlockGuard
{
public:
    FlockGuard(int fd);
    FlockGuard(std::filesystem::path path);
    ~FlockGuard();
};
} // namespace linglong::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_LOCK_H */
