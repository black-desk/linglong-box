#ifndef LINGLONG_BOX_SRC_UTIL_STRING_H_
#define LINGLONG_BOX_SRC_UTIL_STRING_H_

#include <cassert>
#include <numeric>
#include <string>
#include <vector>

namespace linglong::box::util {

template <typename T>
inline std::string strjoin(const std::vector<T>& strs, const std::string_view &seq = " ")
{
    assert(strs.size());

    std::string init(strs.front());

    if (strs.size() == 1) {
        return std::move(init);
    }

    return std::accumulate(strs.begin() + 1, strs.end(), init,
                           [&seq](std::string &x, const std::string_view &y) -> std::string & {
                               x += seq;
                               return x += y;
                           });
}

} // namespace linglong::box::util

#endif /* LINGLONG_BOX_SRC_UTIL_STRING_H_ */
