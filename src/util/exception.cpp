#include "exception.h"

namespace linglong::util {

RuntimeError::RuntimeError(std::string &&s)
    : std::runtime_error("")
    , msg(std::move(s))
{
}

const char *RuntimeError::what() const noexcept
{
    return this->msg.c_str();
}

} // namespace linglong::util
