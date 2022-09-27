#ifndef LINGLONG_BOX_SRC_UTIL_LOG_H_
#define LINGLONG_BOX_SRC_UTIL_LOG_H_

#include <array>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/syslog_sink.h"
#include "spdlog/spdlog.h"
#include "util/constexpr_map.h"

namespace linglong::box::util {

inline void init_logger(const std::string &name)
{
    // Initialize logger

    auto syslog = std::make_shared<spdlog::sinks::syslog_sink_st>(name, LOG_PID, LOG_USER, true);
    auto stdlog = std::make_shared<spdlog::sinks::stdout_color_sink_st>();

    syslog->set_pattern("<%s/%!> %#: %v");

    std::array<spdlog::sink_ptr, 2> sinks = {
        syslog,
        stdlog,
    };

    auto logger = std::make_shared<spdlog::logger>(name, begin(sinks), end(sinks));
    spdlog::set_default_logger(move(logger));

    // Initialize logger level
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
        spdlog::set_level(map.at(level ? level : "off"));
    } catch (const std::range_error &e) {
        spdlog::set_level(spdlog::level::off);
    }
}

} // namespace linglong::box::util

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_LOG_H_ */
