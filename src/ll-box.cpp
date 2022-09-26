#include <cassert>
#include <cstdlib>

#include "ll-box.h"
#include "commands.h"
#include "util/exception.h"
#include "util/log.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/syslog_sink.h"
#include "docopt.cpp/docopt.h"

namespace linglong::box {

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
        auto syslog_logger = spdlog::syslog_logger_mt("syslog", "ll-box", LOG_PID);
        spdlog::set_default_logger(std::move(syslog_logger));
        util::init_log_level();
    }

    std::string usage;

    for (auto &command : commands) {
        auto commandUsages = std::get<1>(command);
        for (auto &commandUsage : commandUsages) {
            usage += "  " + commandUsage + "\n";
        }
    }
    usage.pop_back();

    usage = fmt::format(USAGE_TEMPLATE, usage);

    SPDLOG_TRACE("generated usage:\n{}", usage);

    std::map<std::string, docopt::value> args = docopt::docopt(usage, {argv + 1, argv + argc}, true, "ll-box 1.0");

    SPDLOG_TRACE("parsed args:\n{}", [&args]() -> std::string {
        std::stringstream buf;
        for (auto &arg : args) {
            buf << arg.first << " " << arg.second << std::endl;
        }
        return buf.str();
    }());

    for (auto &command : commands) {
        if (args.find(std::get<0>(command))->second.asBool()) {
            try {
                std::get<2>(command)(args);
                return 0;
            } catch (const std::exception &e) {
                std::stringstream buf;
                linglong::util::printException(buf, e);
                std::cerr << fmt::format("ll-box: command \"{}\" execution failed: {}", std::get<0>(command),
                                         buf.str());
                return -1;
            }
        }
    }

    // NOTE: code should never run to here
    assert(false);
    std::cerr << fmt::format("ll-box: unknown command");
    return -1;
}

} // namespace linglong::box
