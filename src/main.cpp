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

#include "commands.h"
#include "util/exception.h"

int main(int argc, char **argv)
{
    auto cmd = showHelpInfomationError;

    if (argc >= 2) {
        std::string command = argv[1];
        if (command == "--help") {
            cmd = showHelpInfomation;
        } else if (command == "create") {
            cmd = create;
        } else if (command == "start") {
            cmd = start;
        } else if (command == "state") {
            cmd = state;
        } else if (command == "kill") {
            cmd = kill;
        } else if (command == "delete") {
            cmd = delete_;
        } else if (command == "list") {
            cmd = list;
        } else if (command == "exec") {
            cmd = exec;
        }
    }

    try {
        cmd(argc, argv);
    } catch (const std::exception &e) {
        std::stringstream buf;
        linglong::util::printException(buf, e);
        std::cerr << "create container failed:" << buf.str();
        showHelpInfomationError(0, nullptr);
    }

    return 0;
}
