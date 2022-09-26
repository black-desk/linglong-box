#ifndef LINGLONG_BOX_SRC_COMMANDS_H
#define LINGLONG_BOX_SRC_COMMANDS_H

#include <vector>

#define DOCOPT_HEADER_ONLY
#include "docopt.cpp/docopt.h"
#undef DOCOPT_HEADER_ONLY

namespace linglong::box {

using docoptArgs = std::map<std::string, docopt::value>;

extern std::vector<std::tuple<std::string, std::vector<std::string>, std::function<void(const docoptArgs &)>>> commands;

} // namespace linglong::box

#endif
