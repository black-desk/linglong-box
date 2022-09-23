/*
 * Copyright (c) 2020-2021. Uniontech Software Ltd. All rights reserved.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Chen Linxuan <chenlinxuan@uniontech.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "init.h"
#include "monitor.h"
#include "rootfs_preparer.h"
#include "ll-box.h"

#include <string>

int main(int argc, char **argv)
{
    auto exe = std::string(argv[0]);
    if (exe == "init") {
        return linglong::box::init(argc, argv);
    } else if (exe == "monitor") {
        return linglong::box::monitor(argc, argv);
    } else if (exe == "rootfs-preparer") {
        return linglong::box::rootfs_preparer(argc, argv);
    } else {
        return linglong::box::ll_box(argc, argv);
    }
}
