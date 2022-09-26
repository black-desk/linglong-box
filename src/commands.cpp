#include <filesystem>
#include <stdexcept>
#include <string>
#include <iostream>
#include <numeric>
#include "spdlog/spdlog.h"

#include "commands.h"
#include "lib/oci/runtime.h"
#include "util/fd.h"
#include "util/string.h"
#include "util/fmt.h"

namespace linglong::box {

// clang-format off

#define USAGE(...) __VA_ARGS__

#define COMMAND(name, usage, X) \
    { \
        #name, \
        { \
            usage \
        }, \
        [](const docoptArgs &args) { \
            try { \
                X; \
            } catch (...) { \
                std::throw_with_nested( \
                    std::runtime_error("execute command \"" #name "\" failed") \
                ); \
            } \
        } \
    }

// clang-format on

using OCI::Runtime;
using std::cout;
using std::endl;
using std::filesystem::path;
using std::function;
using std::move;
using std::string;
using std::tuple;
using std::vector;
using util::FD;

vector<tuple<string, vector<string>, function<void(const docoptArgs &)>>> commands = {

    COMMAND(create, USAGE("ll-box create <id> <path_to_bundle>"),
            {
                SPDLOG_TRACE("ll-box create called");

                auto containerID = args.find("<id>")->second.asString();
                path pathToBundle = args.find("<path_to_bundle>")->second.asString();

                SPDLOG_TRACE("[container id=\"{}\", bundle=\"{}\"]", containerID, pathToBundle);

                auto bundle = FD(pathToBundle, O_PATH);

                Runtime r;
                r.Create(containerID, move(bundle));
            }),

    COMMAND(start, USAGE("ll-box start [(-i|--console-socket=<address>) --extra-fds=<nfd> --box-as-init] <id>"),
            {
                SPDLOG_TRACE("ll-box start called");

                auto containerID = args.find("<id>")->second.asString();
                auto interactive = args.find("-i")->second.asBool();
                auto consoleSocket = args.find("--console-socket")->second.asString();
                auto extraFDs = args.find("--extra-fds")->second.asLong();
                auto boxAsInit = args.find("--box-as-init")->second.asBool();

                SPDLOG_TRACE(
                    "[container id=\"{}\", interactive={}, console socket=\"{}\", extra fds={}, box as init={}]",
                    containerID, interactive, consoleSocket, extraFDs, boxAsInit);

                Runtime r;
                r.Start(containerID, interactive, consoleSocket, extraFDs, boxAsInit);
            }),

    COMMAND(
        exec,
        USAGE("ll-box exec [(-i|--console-socket=<address>) --extra-fds=<nfd>] <id> -p <path_to_json>",
              "ll-box exec [(-i|--console-socket=<address>) --extra-fds=<nfd>] <id> -- <command>..."),
        {
            SPDLOG_TRACE("ll-box exec called");

            auto containerID = args.find("<id>")->second.asString();
            bool interactive = args.find("-i")->second.asBool();
            auto consoleSocket = args.find("--console-socket")->second.asString();
            int extraFDs = args.find("--extra-fds")->second.asLong();

            Runtime r;

            if (args.find("-p")->second.asBool()) {
                path pathToJson = args.find("<path_to_json>")->second.asString();

                SPDLOG_TRACE(
                    "[container id=\"{}\", interactive={}, console socket=\"{}\", extra fds={}, path to json=\"{}\"]",
                    containerID, interactive, consoleSocket, extraFDs, pathToJson.string());

                auto process = FD(::open(pathToJson.c_str(), O_PATH));
                r.Exec(containerID, move(process), interactive, consoleSocket, extraFDs);
            } else {
                auto cmd = args.find("<command>")->second.asStringList();

                SPDLOG_TRACE(
                    "[container id=\"{}\", interactive={}, console socket=\"{}\", extra fds={}, command=\"{}\"]",
                    containerID, interactive, consoleSocket, extraFDs, util::strjoin(cmd));

                r.Exec(containerID, cmd, interactive, consoleSocket, extraFDs);
            }
        }),

    COMMAND(kill, USAGE("ll-box kill <id> <signal>"),
            {
                SPDLOG_TRACE("ll-box kill called");

                auto containerID = args.find("<id>")->second.asString();
                int signal = args.find("<signal>")->second.asLong();

                SPDLOG_TRACE("[container id=\"{}\", signal={}]", containerID, signal);

                Runtime r;
                r.Kill(containerID, signal);
            }),

    COMMAND(state, USAGE("ll-box state <id>"),
            {
                SPDLOG_TRACE("ll-box state called");

                auto containerID = args.find("<id>")->second.asString();

                SPDLOG_TRACE("[container id=\"{}\"]", containerID);

                Runtime r;
                auto state = r.State(containerID);
                cout << state << std::endl;
            }),

    COMMAND(list, USAGE("ll-box list"),
            {
                SPDLOG_TRACE("ll-box list called");

                Runtime r;
                auto list = r.List();
                for (auto &container : list) {
                    cout << container << endl;
                }
            }),

    COMMAND(delete, USAGE("ll-box delete <id>"),
            {
                SPDLOG_TRACE("ll-box delete called");

                auto containerID = args.find("<id>")->second.asString();

                SPDLOG_TRACE("[container id=\"{}\"]", containerID);

                Runtime r;
                r.Delete(containerID);
            }),
};

} // namespace linglong::box
