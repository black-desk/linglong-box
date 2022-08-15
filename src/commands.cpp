#include <stdexcept>
#include <streambuf>
#include <string>
#include <iostream>
#include <sstream>

#include "oci/runtime.h"

#include "commands.h"

static void doShowHelpInfomation(bool) noexcept;

void showHelpInfomation(int argc, char **argv)
{
    doShowHelpInfomation(false);
}

void showHelpInfomationError(int argc, char **argv)
{
    doShowHelpInfomation(true);
}

static std::string helpInfomation = R"(OCI runtime for linglong.

  Usage: ll-box create CONTAINER_ID [PATH_TO_BUNDLE]
         ll-box start [-d] CONTAINER_ID
         ll-box exec [-d] CONTAINER_ID -p PATH_TO_PROCESS_JSON
         ll-box exec [-d] CONTAINER_ID -- COMMAND
         ll-box stop CONTAINER_ID
         ll-box state CONTAINER_ID
         ll-box kill CONTAINER_ID
         ll-box delete CONTAINER_ID
         ll-box --help # show this help information)";

static std::runtime_error unexpectedCommandLineArgumentsError("unexpected command line arguments");

static void doShowHelpInfomation(bool isError) noexcept
{
    auto *out = isError ? &std::cerr : &std::cout;

    *out << helpInfomation << std::endl;

    if (isError) {
        exit(-1);
    }
}

// TODO: option should not be parsed manually.

void create(int argc, char **argv)
{
    // ll-box create CONTAINER_ID [PATH_TO_BUNDLE]

    if (std::string(argv[1]) != "create" || argc < 3 || argc > 4) {
        throw unexpectedCommandLineArgumentsError;
    }

    std::string containerID(argv[2]);

    // default to use "." as bundle path
    std::string pathToBundle(argc > 4 ? argv[3] : ".");

    linglong::OCI::Runtime r;

    r.Create(containerID, pathToBundle);

    return;
}

void start(int argc, char **argv)
{
    // ll-box start [-i] CONTAINER_ID

    if (std::string(argv[1]) != "start" || argc < 3) {
        throw unexpectedCommandLineArgumentsError;
    }

    bool interactive = false;
    if (std::string(argv[2]) == "-i") {
        interactive = true;
    }

    int optionOffset = interactive;

    std::string containerID(argv[2 + optionOffset]);

    linglong::OCI::Runtime r;

    r.Start(containerID, interactive);
}

void kill(int argc, char **argv)
{
    // ll-box kill CONTAINER_ID
    if (std::string(argv[1]) != "kill" || argc != 3) {
        throw unexpectedCommandLineArgumentsError;
    }

    std::string containerID(argv[2]);

    linglong::OCI::Runtime r;

    r.Kill(containerID);
}

void delete_(int argc, char **argv)
{
    // ll-box delete CONTAINER_ID

    if (std::string(argv[1]) != "delete" || argc != 3) {
        throw unexpectedCommandLineArgumentsError;
    }

    std::string containerID(argv[2]);

    linglong::OCI::Runtime r;

    r.Delete(containerID);
}

void list(int argc, char **argv)
{
    // ll-box list

    if (std::string(argv[1]) != "kill" || argc != 2) {
        throw unexpectedCommandLineArgumentsError;
    }

    linglong::OCI::Runtime r;

    auto containerIDs = r.List();

    for (auto const &containerID : containerIDs) {
        std::cout << containerID << std::endl;
    }
}

void state(int argc, char **argv)
{
    // ll-box qurey CONTAINER_ID

    if (std::string(argv[1]) != "state" || argc != 3) {
        throw unexpectedCommandLineArgumentsError;
    }

    std::string containerID(argv[2]);

    linglong::OCI::Runtime r;

    auto state = r.State(containerID);

    std::cout << state << std::endl;
}

void exec(int argc, char **argv)
{
    // ll-box exec [-d] CONTAINER_ID -p PATH_TO_PROCESS_JSON
    // ll-box exec [-d] CONTAINER_ID -- COMMAND

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
}
