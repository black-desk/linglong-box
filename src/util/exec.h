#ifndef LINGLONG_BOX_SRC_UTIL_EXEC_H_
#define LINGLONG_BOX_SRC_UTIL_EXEC_H_

#include "spdlog/spdlog.h"
#include <filesystem>
#include <functional>
#include <string>
#include <unistd.h>
#include <vector>
#include "util/string.h"
#include "util/fmt.h"

namespace linglong::box::util {

inline pid_t exec(const std::filesystem::path &exe, const std::vector<std::string> &comm, std::function<void()> child,
                  const std::vector<std::string> &env = {})
{
    SPDLOG_TRACE("linglong::box::util::exec called");
    SPDLOG_TRACE("[exe=\"{}\", comm=\"{}\", env=\"{}\"]", exe, util::strjoin(comm), util::strjoin(env));

    auto argv = std::unique_ptr<const char *[]>(new const char *[comm.size() + 1]);

    for (int i = 0; i < comm.size(); i++) {
        argv[i] = comm[i].c_str();
    }
    argv[comm.size()] = nullptr;

    auto environs = std::unique_ptr<const char *[]>(new const char *[env.size() + 1]);
    for (int i = 0; i < env.size(); i++) {
        environs[i] = env[i].c_str();
    }
    environs[env.size()] = nullptr;

    bool failed = false;

    auto ret = vfork();

    if (ret) {
        if (failed) {
            throw fmt::system_error(errno, "exec \"{}\" failed", exe);
        }
        return ret;
    } else {
        try {
            child();
        } catch (...) {
            SPDLOG_ERROR("child() throw before exec \"{}\" ", exe.c_str());
            failed = true;
            if (errno != 0) {
                exit(errno);
            } else {
                exit(-1);
            }
        }
        try {
            ::execve(exe.c_str(), (char **)argv.get(), env.size() == 0 ? environ : (char **)environs.get());
            failed = true;
            throw fmt::system_error(errno, "exec \"{}\" failed", exe);
        } catch (const std::exception &e) {
            SPDLOG_ERROR(e.what());
        } catch (...) {
            SPDLOG_CRITICAL("UNEXPECTED exec \"{}\" failed", exe);
        }
        exit(-1);
    }
}

} // namespace linglong::box::util

#endif
