#ifndef LINGLONG_BOX_SRC_UTIL_EXCEPTION_H
#define LINGLONG_BOX_SRC_UTIL_EXCEPTION_H

#include <exception>
#include <sstream>

namespace linglong::box::util {

inline void doNestWhat(std::ostream &out, const std::exception &e, int depth = 0) noexcept
{
    using namespace std;

    for (int i = 1; i <= depth; i++) {
        out << "  ";
    }
    out << e.what() << endl;
    try {
        std::rethrow_if_nested(e);
    } catch (const exception &nested) {
        doNestWhat(out, nested, depth + 1);
    }
}

inline std::string nestWhat(const std::exception &e) noexcept
{
    using namespace std;

    stringstream buf;

    doNestWhat(buf, e);

    return buf.str();
}

} // namespace linglong::box::util

#endif
