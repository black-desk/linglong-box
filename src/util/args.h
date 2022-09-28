#ifndef LINGLONG_BOX_SRC_UTIL_ARGS_H_
#define LINGLONG_BOX_SRC_UTIL_ARGS_H_

#include <map>
#include <string>

#include "util/log.h"
#include "docopt.cpp/docopt.h"

namespace linglong::box::util {

inline std::map<std::string, docopt::value>
parseArgs(const char USAGE[], int argc, char **argv, bool help = true,
          const std::string &version = {})
{
    auto args = docopt::docopt(USAGE, {argv + 1, argv + argc}, help,version);

    SPDLOG_TRACE("parsed args:\n{}", [&args]() noexcept -> std::string {
        std::stringstream buf;
        for (auto &arg : args) {
            buf << arg.first << " " << arg.second << std::endl;
        }
        auto str = buf.str();
        str.pop_back();
        return str;
    }());

    return std::move(args);
}

} // namespace linglong::box::util

#endif
