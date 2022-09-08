#ifndef LINGLONG_BOX_SRC_UTIL_COMMON_H
#define LINGLONG_BOX_SRC_UTIL_COMMON_H

class NonCopyable
{
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &A) = delete;
};

#endif
