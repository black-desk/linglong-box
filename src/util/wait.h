#ifndef LINGLONG_BOX_SRC_UTIL_WAIT_H
#define LINGLONG_BOX_SRC_UTIL_WAIT_H

#include <optional>
#include <tuple>

#include <wait.h>

#include <spdlog/spdlog.h>

namespace linglong::util {

// (return code, terminate signal)
inline std::pair<bool, int> parse_wstatus(const int &wstatus)
{
    if (WIFEXITED(wstatus)) {
        auto code = WEXITSTATUS(wstatus);
        return {false, code};
    } else if (WIFSIGNALED(wstatus)) {
        return {true, WTERMSIG(wstatus)};
    } else {
        // FIXME:
        throw std::runtime_error(fmt::format("unexpected wstatus={}", wstatus));
    }
}

} // namespace linglong::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_WAIT_H */
