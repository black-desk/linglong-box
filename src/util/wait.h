#ifndef LINGLONG_BOX_SRC_UTIL_WAIT_H
#define LINGLONG_BOX_SRC_UTIL_WAIT_H

#include <optional>
#include <tuple>

#include <wait.h>

#include <spdlog/spdlog.h>

namespace linglong::util {

// (return code, terminate signal)
inline std::pair<std::optional<int>, std::optional<int>> parse_wstatus(const int &wstatus)
{
    if (WIFEXITED(wstatus)) {
        auto code = WEXITSTATUS(wstatus);
        return {code, std::nullopt};
    } else if (WIFSIGNALED(wstatus)) {
        return {std::nullopt, WTERMSIG(wstatus)};
    } else {
        // FIXME: should we handle STOP?
        spdlog::error("unhandled wstatus {}", wstatus);
        return {std::nullopt, std::nullopt};
    }
}

} // namespace linglong::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_WAIT_H */
