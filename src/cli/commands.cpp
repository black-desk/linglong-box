#include <filesystem>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <iostream>
#include <sstream>

#include "oci/runtime.h"

#include "commands.h"

namespace linglong::box {

#define USAGE(...) \
    { \
        __VA_ARGS__ \
    }

#define COMMAND(name, usage, X) \
    { \
#name,usage,[](const docoptArgs &args){try X catch(...) {std::throw_with_nested(std::runtime_error("command name failed"));}} \
    }

std::vector<std::tuple<std::string, std::vector<std::string>, std::function<void(const docoptArgs &)>>> commands = {
    COMMAND(create, USAGE("ll-box create <id> [<path_to_bundle>]"),
            {
                std::string containerID = args.find("<id>")->second.asString();
                std::filesystem::path pathToBundle = std::filesystem::current_path();
                {
                    auto arg = args.find("<path_to_bundle>");
                    if (arg != args.end()) {
                        pathToBundle = arg->second.asString();
                    }
                }
                linglong::box::OCI::Runtime r;
                r.Create(containerID, pathToBundle);
            }),
    COMMAND(start, USAGE("ll-box start [(-i | --console-socket=<address>) --extra-fds=<nfd> --box-as-init] <id>"),
            {
                std::string containerID = args.find("<id>")->second.asString();
                bool interactive = false;
                {
                    auto arg = args.find("-i");
                    if (arg != args.end()) {
                        interactive = arg->second.asBool();
                    }
                }
                std::string consoleSocket = "";
                {
                    auto arg = args.find("--console-socket");
                    if (arg != args.end()) {
                        consoleSocket = arg->second.asString();
                    }
                }
                int extraFDs = args.find("--extra-fds")->second.asLong();
                bool boxAsInit = args.find("--box-as-init")->second.asBool();

                linglong::box::OCI::Runtime r;
                r.Start(containerID, interactive, consoleSocket, extraFDs, boxAsInit);
            }),
    COMMAND(exec,
            USAGE("ll-box exec [(-i | --console-socket=<address>) --extra-fds=<nfd>] <id> -p <path_to_json>",
                  "ll-box exec [(-i | --console-socket=<address>) --extra-fds=<nfd>] <id> [--] <command>..."),
            {
                std::string containerID = args.find("<id>")->second.asString();
                bool interactive = false;
                {
                    auto arg = args.find("-i");
                    if (arg != args.end()) {
                        interactive = arg->second.asBool();
                    }
                }
                std::string consoleSocket = "";
                {
                    auto arg = args.find("--console-socket");
                    if (arg != args.end()) {
                        consoleSocket = arg->second.asString();
                    }
                }
                int extraFDs = args.find("--extra-fds")->second.asLong();

                linglong::box::OCI::Runtime r;

                if (args.find("-p")->second.asBool()) {
                    std::filesystem::path pathToJson = args.find("<path_to_json>")->second.asString();
                    r.Exec(containerID, pathToJson, interactive, consoleSocket, extraFDs);
                } else {
                    auto cmd = args.find("commands")->second.asStringList();
                    r.Exec(containerID, cmd, interactive, consoleSocket, extraFDs);
                }
            }),
    COMMAND(kill, USAGE("ll-box kill <id> [<signal>]"),
            {
                std::string containerID = args.find("<id>")->second.asString();
                int signal = args.find("<signal>")->second.asLong();

                linglong::box::OCI::Runtime r;
                r.Kill(containerID, signal);
            }),
    COMMAND(state, USAGE("ll-box state <id>"),
            {
                std::string containerID = args.find("<id>")->second.asString();

                linglong::box::OCI::Runtime r;
                std::string state = r.State(containerID);
                std::cout << state << std::endl;
            }),
    COMMAND(list, USAGE("ll-box list"),
            {
                linglong::box::OCI::Runtime r;
                auto list = r.List();
                for (auto &container : list) {
                    std::cout << container << std::endl;
                }
            }),
    COMMAND(delete, USAGE("ll-box delete <id>"),
            {
                std::string containerID = args.find("<id>")->second.asString();
                linglong::box::OCI::Runtime r;
                r.Delete(containerID);
            }),
};
} // namespace linglong::box
