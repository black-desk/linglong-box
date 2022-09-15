#include "filesystem.h"
#include <fcntl.h>
#include <fmt/format.h>

namespace linglong::util::fs {
void touch(std::filesystem::path path, __mode_t mode)
{
    try {
        auto fd = util::FD(open(path.c_str(), O_CREAT, mode));
    } catch (...) {
        std::throw_with_nested(fmt::system_error(errno, "failed to open{}", path));
    }
}
} // namespace linglong::util::fs
