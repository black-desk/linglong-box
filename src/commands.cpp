#include <filesystem>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "spdlog/spdlog.h"

// #include "oci/runtime.h"

#include "commands.h"

namespace linglong::box {

#define USAGE(...) \
    { \
        __VA_ARGS__ \
    }

#define COMMAND(name, usage, X) \
    { \
#name,usage,[](const docoptArgs &args){try X catch(...) {std::throw_with_nested(std::runtime_error("command "#name" failed"));}} \
    }

std::vector<std::tuple<std::string, std::vector<std::string>, std::function<void(const docoptArgs &)>>> commands = {
    COMMAND(create, USAGE("ll-box create <id> [<path_to_bundle>]"),
            {
                std::string containerID = args.find("<id>")->second.asString();
                std::filesystem::path pathToBundle = std::filesystem::current_path();
                {
                    auto arg = args.find("<path_to_bundle>");
                    if (arg->second.kind() != docopt::Kind::Empty) {
                        pathToBundle = arg->second.asString();
                    }
                }

                SPDLOG_TRACE("ll-box create called [container id=\"{}\", bundle=\"{}\"", containerID,
                             pathToBundle.string());
                // linglong::box::OCI::Runtime r;
                // r.Create(containerID, pathToBundle);
            }),
    COMMAND(
        start, USAGE("ll-box start [(-i | --console-socket=<address>) --extra-fds=<nfd> --box-as-init] <id>"),
        {
            std::string containerID = args.find("<id>")->second.asString();
            bool interactive = false;
            {
                auto arg = args.find("-i");
                if (arg->second.kind() != docopt::Kind::Empty) {
                    interactive = arg->second.asBool();
                }
            }
            std::string consoleSocket = "";
            {
                auto arg = args.find("--console-socket");
                if (arg->second.kind() != docopt::Kind::Empty) {
                    consoleSocket = arg->second.asString();
                }
            }
            int extraFDs = 0;
            {
                auto arg = args.find("--extra-fds");
                if (arg->second.kind() != docopt::Kind::Empty) {
                    consoleSocket = arg->second.asLong();
                }
            }
            bool boxAsInit = args.find("--box-as-init")->second.asBool();

            SPDLOG_TRACE(
                "ll-box start called [container id=\"{}\", interactive={}, console socket=\"{}\", extra fds={}, box as init={}",
                containerID, interactive, consoleSocket, extraFDs, boxAsInit);
            // linglong::box::OCI::Runtime r;
            // r.Start(containerID, interactive, consoleSocket, extraFDs, boxAsInit);
        }),
    COMMAND(
        exec,
        USAGE("ll-box exec [(-i | --console-socket=<address>) --extra-fds=<nfd>] <id> -p <path_to_json>",
              "ll-box exec [(-i | --console-socket=<address>) --extra-fds=<nfd>] <id> -- <command>..."),
        {
            std::string containerID = args.find("<id>")->second.asString();
            bool interactive = false;
            {
                auto arg = args.find("-i");
                if (arg->second.kind() != docopt::Kind::Empty) {
                    interactive = arg->second.asBool();
                }
            }
            std::string consoleSocket = "";
            {
                auto arg = args.find("--console-socket");
                if (arg->second.kind() != docopt::Kind::Empty) {
                    consoleSocket = arg->second.asString();
                }
            }
            int extraFDs = args.find("--extra-fds")->second.asLong();

            // linglong::box::OCI::Runtime r;

            if (args.find("-p")->second.asBool()) {
                std::filesystem::path pathToJson = args.find("<path_to_json>")->second.asString();
                SPDLOG_TRACE(
                    "ll-box exec called [container id=\"{}\", interactive={}, console socket=\"{}\", extra fds={}, path to json=\"{}\"]",
                    containerID, interactive, consoleSocket, extraFDs, pathToJson.string());
                // r.Exec(containerID, pathToJson, interactive, consoleSocket, extraFDs);
            } else {
                auto cmd = args.find("commands")->second.asStringList();
                SPDLOG_TRACE(
                    "ll-box exec called [container id=\"{}\", interactive={}, console socket=\"{}\", extra fds={}, command=\"{}\"]",
                    containerID, interactive, consoleSocket, extraFDs, std:: cmd);
                // r.Exec(containerID, cmd, interactive, consoleSocket, extraFDs);
            }
        }),
    COMMAND(kill, USAGE("ll-box kill <id> [<signal>]"),
            {
                std::string containerID = args.find("<id>")->second.asString();
                int signal = args.find("<signal>")->second.asLong();

                SPDLOG_TRACE("ll-box kill called [container id=\"{}\", signal={}]", containerID, signal);
                // linglong::box::OCI::Runtime r;
                // r.Kill(containerID, signal);
            }),
    COMMAND(state, USAGE("ll-box state <id>"),
            {
                std::string containerID = args.find("<id>")->second.asString();

                SPDLOG_TRACE("ll-box state called [container id=\"{}\"]", containerID);
                // linglong::box::OCI::Runtime r;
                // std::string state = r.State(containerID);
                // std::cout << state << std::endl;
            }),
    COMMAND(list, USAGE("ll-box list"),
            {
                SPDLOG_TRACE("ll-box list called");
                // linglong::box::OCI::Runtime r;
                // auto list = r.List();
                // for (auto &container : list) {
                // std::cout << container << std::endl;
                // }
            }),
    COMMAND(delete, USAGE("ll-box delete <id>"),
            {
                std::string containerID = args.find("<id>")->second.asString();

                SPDLOG_TRACE("ll-box delete called [container id=\"{}\"]", containerID);
                // linglong::box::OCI::Runtime r;
                // r.Delete(containerID);
            }),
};
} // namespace linglong::box
