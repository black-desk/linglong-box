/*
 * Copyright (c) 2020-2021. Uniontech Software Ltd. All rights reserved.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Chen Linxuan <chenlinxuan@uniontech.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <iostream>
#include <string>
#include <sstream>

#include <spdlog/spdlog.h>

#include "commands.h"
#include "init.h"
#include "monitor.h"
#include "util/exception.h"

int main(int argc, char **argv)
{
    if (std::string(argv[0]) == "init") {
        init(std::atoi(argv[1]), std::atoi(argv[2]), std::atoi(argv[3]));
        return 0;
    } else if (std::string(argv[0]) == "monitor") {
        monitor(argv[1]);
        return 0;
    }

    auto cmd = showHelpInfomationError;

    if (argc >= 2) {
        std::string command = argv[1];
        if (command == "create") {
            cmd = create;
        } else if (command == "start") {
            cmd = start;
        } else if (command == "kill") {
            cmd = kill;
        } else if (command == "delete") {
            cmd = delete_;
        } else if (command == "list") {
            cmd = list;
        } else if (command == "state") {
            cmd = state;
        } else if (command == "exec") {
            cmd = exec;
        } else if (command == "--help") {
            cmd = showHelpInfomation;
        }
    }

    try {
        cmd(argc, argv);
    } catch (const std::exception &e) {
        std::stringstream buf;
        linglong::util::printException(buf, e);
        spdlog::error("ll-box: command execution failed: {}", buf);
        showHelpInfomationError(0, nullptr);
    }

    return 0;
}
