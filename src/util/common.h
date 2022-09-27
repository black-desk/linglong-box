#ifndef LINGLONG_BOX_SRC_UTIL_COMMON_H
#define LINGLONG_BOX_SRC_UTIL_COMMON_H

#include <type_traits>

namespace linglong::box::util {

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

template<typename T>
class Singleton : virtual public NonCopyable
{
public:
    T &instance()
    {
        static T instance;
        return instance;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
};

} // namespace linglong::box::util

#endif
