#ifndef LINGLONG_BOX_SRC_UTIL_COMMON_H
#define LINGLONG_BOX_SRC_UTIL_COMMON_H

namespace linglong::util {

class NonCopyable
{
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
};

class NonMoveable
{
public:
    NonMoveable() = default;
    NonMoveable(NonMoveable &&) = delete;
    NonMoveable &operator=(NonMoveable &&) = delete;
};

} // namespace linglong::util

#endif
