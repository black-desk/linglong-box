#include <filesystem>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <iostream>
#include <sstream>

#include "oci/runtime.h"

#include "commands.h"

namespace linglong::box {

#define COMMAND(name, X) \
    { \
#name,[](const docoptArgs &args){try X catch(...){std::throw_with_nested(std::runtime_error("command name failed"));}} \
    }

std::vector<std::pair<std::string, std::function<void(const docoptArgs &)>>> commands = {
    COMMAND(create,
            {
                linglong::box::OCI::Runtime r;
                std::string containerID = args.find("<container_id>")->second.asString();
                std::filesystem::path pathToBundle = args.find("<path_to_bundle>")->second.asString();
                if (pathToBundle.empty()) {
                    pathToBundle = std::filesystem::current_path();
                }
                r.Create(containerID, pathToBundle);
            }),
    COMMAND(start, {
                    linglong::box::OCI::Runtime r;
                std::string containerID = args.find("<container_id>")->second.asString();


        r.Start(containerID, interactive);

                    }),
};

void start(int argc, char **argv)
{
    // ll-box start [-i] CONTAINER_ID

    try {
    } catch (...) {
        std::throw_with_nested(std::runtime_error("command start failed"));
    }
}

void kill(int argc, char **argv)
{
    // ll-box kill CONTAINER_ID

    try {
        if (std::string(argv[1]) != "kill" || argc != 3) {
            throw unexpectedCommandLineArgumentsError;
        }

        std::string containerID(argv[2]);

        linglong::OCI::Runtime r;

        r.Kill(containerID);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("command kill failed"));
    }
}

void delete_(int argc, char **argv)
{
    // ll-box delete CONTAINER_ID

    try {
        if (std::string(argv[1]) != "delete" || argc != 3) {
            throw unexpectedCommandLineArgumentsError;
        }

        std::string containerID(argv[2]);

        linglong::OCI::Runtime r;

        r.Delete(containerID);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("command delete failed"));
    }
}

void list(int argc, char **argv)
{
    // ll-box list

    try {
        if (std::string(argv[1]) != "kill" || argc != 2) {
            throw unexpectedCommandLineArgumentsError;
        }

        linglong::OCI::Runtime r;

        auto containerIDs = r.List();

        for (auto const &containerID : containerIDs) {
            std::cout << containerID << std::endl;
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("command list failed"));
    }
}

void state(int argc, char **argv)
{
    // ll-box qurey CONTAINER_ID

    try {
        if (std::string(argv[1]) != "state" || argc != 3) {
            throw unexpectedCommandLineArgumentsError;
        }

        std::string containerID(argv[2]);

        linglong::OCI::Runtime r;

        auto state = r.State(containerID);

        std::cout << state << std::endl;
    } catch (...) {
        std::throw_with_nested(std::runtime_error("command state failed"));
    }
}

void exec(int argc, char **argv)
{
    // ll-box exec [-d] CONTAINER_ID -p PATH_TO_PROCESS_JSON
    // ll-box exec [-d] CONTAINER_ID -- COMMAND

    try {
        if (std::string(argv[1]) != "exec" || argc < 3) {
            throw unexpectedCommandLineArgumentsError;
        }

        bool detach = false;
        if (std::string(argv[2]) == "-d") {
            detach = true;
        }

        int optionOffset = detach;

        if (argc < 5 + optionOffset) {
            throw unexpectedCommandLineArgumentsError;
        }

        std::string containerID(argv[2 + optionOffset]), spliter(argv[3 + optionOffset]);

        linglong::OCI::Runtime r;

        if (spliter == "-p") { // ll-box exec [-d] CONTAINER_ID -p PATH_TO_PROCESS_JSON

            std::string pathToProcessJson(argv[4 + optionOffset]);

            r.Exec(containerID, pathToProcessJson, detach);

        } else if (spliter == "--") { // ll-box exec [-d] CONTAINER_ID -- COMMAND

            std::vector<std::string> commandToExec;

            for (int i = 4 + optionOffset; i < argc; i++) {
                commandToExec.push_back(std::string(argv[i]));
            }

            r.Exec(containerID, commandToExec, detach);

        } else {
            throw unexpectedCommandLineArgumentsError;
        }

    } catch (...) {
        std::throw_with_nested(std::runtime_error("command exec failed"));
    }
}
} // namespace linglong::box
