#include "util.h"
#include "util/fd.h"

#include <fmt/format.h>

namespace linglong::OCI {

pid_t execProcess(const OCI::Config::Process &process)
{
    util::Pipe sync;
    int appPID = fork();
    if (appPID) { // parent
        sync << 0;
        int ret;
        sync >> ret;
        if (ret) {
            throw std::runtime_error(
                fmt::format("execve \"{}\" failed: {}", nlohmann::json(process).dump(), strerror(errno)));
        }
        return appPID;
    } else {
        int ret;
        sync >> ret;
        int fdlimit = (int)sysconf(_SC_OPEN_MAX);
        for (int i = STDERR_FILENO + 1; i < fdlimit; i++)
            close(i);

        const auto &processArgs = process.args;
        const auto &processEnv = process.env.value_or(std::vector<std::string>());
        const char *args[processArgs.size() + 1];
        const char *env[processEnv.size() + 1];

        for (int i = 0; i < processArgs.size(); i++) {
            args[i] = processArgs[i].c_str();
        }
        args[processArgs.size()] = nullptr;

        for (int i = 0; i < processEnv.size(); i++) {
            env[i] = processEnv[i].c_str();
        }
        env[processEnv.size()] = nullptr;

        ret = execve(args[0], const_cast<char *const *>(args), const_cast<char *const *>(env));
        sync << errno;
        exit(-1);
    }
}
} // namespace linglong::OCI
