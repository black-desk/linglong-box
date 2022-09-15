#ifndef LINGLONG_UTIL_TERMINAL_H
#define LINGLONG_UTIL_TERMINAL_H

#include <memory>
#include <optional>

#include "util/fd.h"
#include "oci/config.h"

#include "fmt/format.h"
#include <sys/fcntl.h>
#include <sys/ioctl.h>

namespace linglong::util {

inline std::shared_ptr<util::FD> setupConsole(std::optional<OCI::Config::Process::ConsoleSize> size)
{

    return pty;
}
} // namespace linglong::util
#endif
