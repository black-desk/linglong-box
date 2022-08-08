/*
 * Copyright (c) 2020-2021. Uniontech Software Ltd. All rights reserved.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>

#include "util/logger.h"
#include "util/oci_runtime.h"
#include "container/container.h"
#include "container/container_option.h"
#include "util/message_reader.h"

extern linglong::Runtime loadBundle(int argc, char **argv);

static std::string helpInfomation = R"(OCI runtime for linglong.

  Usage: ll-box ./some_oci_config.json # load json mode
         ll-box [pipe] [local_to_passing_fd] # pipe mode
         ll-box [container_id] [bundle_root] [command_to_exec] # load bundle mode
         ll-box --help # show this help information)";

void showHelpInfomation(bool err)
{
    auto &out = err ? std::cerr : std::cout;
    out << helpInfomation << std::endl;
    return;
}

int main(int argc, char **argv)
{
    // TODO(iceyer): move loader to ll-loader?
    // NOTE(clx): just comment out loadBundle source for now, as currently unused.
    // bool is_load_bundle = (argc == 4);

    linglong::Option opt;

    if (argc == 2) {
        if (strcmp("--help", argv[1])) {
            showHelpInfomation(false);
            return 0;
        }
    } else if (argc > 4) {
        showHelpInfomation(true);
        return -1;
    }

    // TODO(iceyer): default in rootless
    if (geteuid() != 0) {
        opt.rootless = true;
    }

    try {
        linglong::Runtime r;

        // if (is_load_bundle) {
        // r = loadBundle(argc, argv);
        // } else {

        int socket = atoi(argv[1]);
        if (socket <= 0) {
            socket = open(argv[1], O_RDONLY | O_CLOEXEC);
        }

        std::unique_ptr<linglong::util::MessageReader> reader(new linglong::util::MessageReader(socket));
        auto json = reader->read();

        r = json.get<linglong::Runtime>();

        // }

        if (linglong::util::fs::exists("/tmp/ll-debug")) {
            std::ofstream origin(linglong::util::format("/tmp/ll-debug/%d.json", getpid()));
            origin << json.dump(4);
            origin.close();
        }

        linglong::Container c(r, std::move(reader));
        return c.Start(opt);
    } catch (const std::exception &e) {
        logErr() << "failed: " << e.what();
        return -1;
    }
}
