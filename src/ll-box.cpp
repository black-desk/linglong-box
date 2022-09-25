#include "ll-box.h"
#include "cli/commands.h"
#include "util/exception.h"

#include "spdlog/spdlog.h"
#define DOCOPT_HEADER_ONLY
#include "docopt.cpp/docopt.h"
#undef DOCOPT_HEADER_ONLY

namespace linglong::box {

static const char USAGE[] = R"(ll-box: OCI Runtime for linglong.

    Usage: 
      ll-box create <container_id> [--notify-socket=<notify_socket_address>] [<path_to_bundle>]
      ll-box start [-i | (-d [--console-socket=<console_socket_address>])] [--extra-fds=<nfd>] [--box-as-init] <container_id>
      ll-box exec <container_id> [-i | (-d [--console-socket=<console_socket_address>])] [--extra-fds=<nfd>] -p <path_to_process_json>
      ll-box exec <container_id> [-i | (-d [--console-socket=<console_socket_address>])] [--extra-fds=<nfd>] [--] <command>...
      ll-box stop <container_id>
      ll-box state <container_id>
      ll-box kill <container_id> [<signal>]
      ll-box delete <container_id>
      ll-box -h | --help
      ll-box -v | --version
    Options:
      -h --help          Show this screen.
      -v --version       Show version.
      -i                 Execute command interactively.
      -d                 Execute command then detach.
      --extra-fds=<nfds> Number of extra fds should be passed to process. [default: 0]
)";

int ll_box(int argc, char **argv)
{
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, {argv + 1, argv + argc}, true, "ll-box 1.0");
    for (auto &command : commands) {
        if (args.find(command.first) != args.end()) {
            try {
                command.second(args);
                return 0;
            } catch (const std::exception &e) {
                std::stringstream buf;
                linglong::util::printException(buf, e);
                spdlog::error("ll-box: command execution failed: {}", buf);
                return -1;
            }
        }
    }
    spdlog::error("ll-box: unknown command");
    return -1;
}

} // namespace linglong::box
