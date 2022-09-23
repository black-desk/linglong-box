#include "ll-box.h"
#include "commands/commands.h"
#include "lib/util/exception.h"

#include <string>
#include <sstream>

#include "spdlog/spdlog.h"

namespace linglong::box {

int ll_box(int argc, char **argv)
{
    auto cmd = linglong::showHelpInfomationError;

    if (argc >= 2) {
        std::string command = argv[1];
        if (command == "create") {
            cmd = linglong::create;
        } else if (command == "start") {
            cmd = linglong::start;
        } else if (command == "kill") {
            cmd = linglong::kill;
        } else if (command == "delete") {
            cmd = linglong::delete_;
        } else if (command == "list") {
            cmd = linglong::list;
        } else if (command == "state") {
            cmd = linglong::state;
        } else if (command == "exec") {
            cmd = linglong::exec;
        } else if (command == "--help") {
            cmd = linglong::showHelpInfomation;
        }
    }

    try {
        cmd(argc, argv);
    } catch (const std::exception &e) {
        std::stringstream buf;
        linglong::util::printException(buf, e);
        spdlog::error("ll-box: command execution failed: {}", buf);
        linglong::showHelpInfomationError(0, nullptr);
    }

    return 0;
}

} // namespace linglong::box
