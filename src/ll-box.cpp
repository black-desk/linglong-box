#include <cassert>
#include <cstdlib>

#include "ll-box.h"
#include "commands.h"
#include "util/exception.h"
#include "util/log.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/syslog_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "docopt.cpp/docopt.h"

namespace linglong::box {

using fmt::format;
using sstream = std::stringstream;
using std::cerr;
using std::endl;
using std::get;
using std::map;
using std::move;
using std::string;

static const char USAGE_TEMPLATE[] = R"(ll-box: OCI Runtime for linglong.

Usage:
{}
  ll-box -h | --help
  ll-box -v | --version

Options:
  -h --help                   Show this screen.
  -v --version                Show version.
  -i                          Execute command interactively.
  --console-socket=<address>  Socket address to recive tty fd of process [default: ].
  --extra-fds=<nfds>          Number of extra fds should be passed to process [default: 0].
  --box-as-init               Let ll-box be the init(pid=1) in container instead of process [default: true].
)";

int ll_box(int argc, char **argv)
{
    {
        // Initialize logger

        auto syslog = std::make_shared<spdlog::sinks::syslog_sink_st>("ll-box", LOG_PID, LOG_USER, true);
        auto stdlog = std::make_shared<spdlog::sinks::stdout_color_sink_st>();

        syslog->set_pattern("<%s/%!> %#: %v");

        std::array<spdlog::sink_ptr, 2> sinks = {
            syslog,
            stdlog,
        };

        auto logger = std::make_shared<spdlog::logger>("ll-box", begin(sinks), end(sinks));
        spdlog::set_default_logger(move(logger));
        util::init_log_level();
    }

    map<string, docopt::value> args;

    {
        string usage;

        for (auto &command : commands) {
            auto commandUsages = get<1>(command);
            for (auto &commandUsage : commandUsages) {
                usage += "  " + commandUsage + "\n";
            }
        }
        usage.pop_back();

        usage = format(USAGE_TEMPLATE, usage);

        SPDLOG_TRACE("generated usage:\n{}", usage);

        args = docopt::docopt(usage, {argv + 1, argv + argc}, true, "ll-box 1.0");
    }

    SPDLOG_TRACE("parsed args:\n{}", [&args]() noexcept -> string {
        sstream buf;
        for (auto &arg : args) {
            buf << arg.first << " " << arg.second << endl;
        }
        return buf.str();
    }());

    for (auto &command : commands) {
        if (args.find(get<0>(command))->second.asBool()) {
            try {
                get<2>(command)(args);
                return 0;
            } catch (const std::exception &e) {
                cerr << format("ll-box: execution failed:\n{}", util::nestWhat(e));
                return -1;
            }
        }
    }

    // NOTE: code should never run to here
    assert(false);
    cerr << fmt::format("ll-box: unknown command");
    return -1;
}

} // namespace linglong::box
