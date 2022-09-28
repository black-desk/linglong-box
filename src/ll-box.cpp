#include <cassert>
#include <cstdlib>

#include "ll-box.h"
#include "commands.h"
#include "util/args.h"
#include "util/exception.h"
#include "util/log.h"

namespace linglong::box {

using fmt::format;
using std::cerr;
using std::get;
using std::map;
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

int ll_box(int argc, char **argv) noexcept
{
    util::init_logger("ll-box");

    SPDLOG_DEBUG("ll-box started");

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

        args = util::parseArgs(usage.c_str(), argc, argv, true, "ll-box 1.0");
    }

    for (auto &command : commands) {
        if (args.find(get<0>(command))->second.asBool()) {
            try {
                get<2>(command)(args);
                return 0;
            } catch (const std::exception &e) {
                cerr << format("ll-box: execution failed:\n{}",
                               util::nestWhat(e));
                return -1;
            }
        }
    }

    // NOTE: code should never run to here
    assert(false);
    cerr << format("ll-box: unknown command");
    return -1;
}

} // namespace linglong::box
