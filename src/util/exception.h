#ifndef LINGLONG_BOX_SRC_UTIL_EXCEPTION_H
#define LINGLONG_BOX_SRC_UTIL_EXCEPTION_H

#include <exception>
#include <ostream>

namespace linglong::util {

class RuntimeError : public std::runtime_error
{
    std::string msg;

public:
    RuntimeError(std::string &&s);

    virtual const char *what() const noexcept override;
};
inline void printException(std::ostream &out, const std::exception &e, int depth = 0)
{
    for (int i = 1; i <= depth; i++) {
        out << "  ";
    }
    out << e.what() << std::endl;
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception &nested) {
        printException(out, nested, depth + 1);
    }
}

} // namespace linglong::util

#endif
