#ifndef LINGLONG_BOX_SRC_UTIL_FMT_H_
#define LINGLONG_BOX_SRC_UTIL_FMT_H_

#include <filesystem>
#include "fmt/format.h"

namespace fmt {

template<>
struct formatter<std::filesystem::path> : formatter<std::string> {
};

} // namespace fmt

#endif
