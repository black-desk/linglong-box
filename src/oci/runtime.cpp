#include <fcntl.h>
#include <fmt/format.h>
#include <unistd.h>

#include "runtime.h"

namespace linglong {
namespace OCI {

static const std::string workingDirTemplate = "/run/user/{}/linglong";

Runtime::Runtime()
    : workingDir(fmt::format(workingDirTemplate, getuid()))
{
}

void Runtime::Create(std::string containerID, std::string pathToBundle)
{
}

void Runtime::Start(std::string containerID)
{
    // FIXME: TODO
}
void Runtime::Kill(std::string containerID)
{
    // FIXME: TODO
}
void Runtime::Delete(std::string containerID)
{
    // FIXME: TODO
}
nlohmann::json Runtime::State(std::string containerID)
{
    // FIXME: TODO
}
std::vector<std::string> Runtime::List()
{
    // FIXME: TODO
}

int Runtime::Exec(std::string containerID, std::string pathToProcess, bool detach)
{
    // FIXME: TODO
}

int Runtime::Exec(std::string containerID, std::vector<std::string> commandToExec, bool detach)
{
    // FIXME: TODO
}

} // namespace OCI
} // namespace linglong
