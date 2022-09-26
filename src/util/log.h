#ifndef LINGLONG_BOX_SRC_UTIL_LOG_H_
#define LINGLONG_BOX_SRC_UTIL_LOG_H_

#include <array>
#include <cstdlib>
#include <string_view>
#include "spdlog/spdlog.h"
#include "util/constexpr_map.h"

namespace linglong::box::util {

inline void init_log_level()
{
    using namespace std::literals::string_view_literals;
    static constexpr std::array<std::pair<std::string_view, spdlog::level::level_enum>,
                                spdlog::level::level_enum::n_levels>
        logLevels {{
            {"trace"sv, spdlog::level::trace},
            {"debug"sv, spdlog::level::debug},
            {"info"sv, spdlog::level::info},
            {"warn"sv, spdlog::level::warn},
            {"err"sv, spdlog::level::err},
            {"critical"sv, spdlog::level::critical},
            {"off"sv, spdlog::level::off},
        }};
    static constexpr auto map = Map<std::string_view, spdlog::level::level_enum, logLevels.size()> {{logLevels}};

    const char *level = std::getenv("LINGLONG_BOX_LOG_LEVEL");

    try {
        spdlog::set_level(map.at(level ? level : ""));
    } catch (const std::range_error &e) {
        spdlog::set_level(spdlog::level::off);
    }
}

} // namespace linglong::box::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_LOG_H_ */
