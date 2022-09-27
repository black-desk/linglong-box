#include <functional>
#include <map>
#include <string>

#include "init.h"
#include "monitor.h"
#include "rootfs_preparer.h"
#include "ll-box.h"
#include "util/constexpr_map.h"

int main(int argc, char **argv)
{
    using namespace std::literals::string_view_literals;
    static constexpr std::array<std::pair<std::string_view, int (*)(int, char **) noexcept>, 3> programs {{
        {"ll-box-monitor", linglong::box::monitor},
        {"ll-box-rootfs-preparer", linglong::box::rootfs_preparer},
        {"ll-box-init", linglong::box::init},
    }};
    static constexpr auto map =
        linglong::box::util::Map<std::string_view, int (*)(int, char **) noexcept, programs.size()> {{programs}};

    auto exe = std::string(argv[0]);
    try {
        auto program = map.at(exe);
        return program(argc, argv);
    } catch (std::range_error) {
        return linglong::box::ll_box(argc, argv);
    }
}
