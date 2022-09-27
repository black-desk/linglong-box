#include <functional>
#include <map>
#include <string>

#include "init.h"
#include "monitor.h"
#include "rootfs_preparer.h"
#include "ll-box.h"

int main(int argc, char **argv)
{
    auto exe = std::string(argv[0]);

    std::map<std::string, std::function<int(int, char **)>> programs = {
        {"ll-box-init", linglong::box::init},
        {"ll-box-monitor", linglong::box::monitor},
        {"ll-box-rootfs-preparer", linglong::box::rootfs_preparer},
    };

    auto program = programs.find(exe);

    if (program != programs.end()) {
        program->second(argc, argv);
    } else {
        return linglong::box::ll_box(argc, argv);
    }
}
