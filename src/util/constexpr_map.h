#ifndef LINGLONG_BOX_SRC_UTIL_CONSTEXPR_MAP
#define LINGLONG_BOX_SRC_UTIL_CONSTEXPR_MAP

#include <algorithm>
#include <array>
#include <stdexcept>

template<typename Key, typename Value, std::size_t Size>
struct Map {
    std::array<std::pair<Key, Value>, Size> data;

    [[nodiscard]] constexpr Value at(const Key &key) const
    {
        const auto itr = std::find_if(begin(data), end(data), [&key](const auto &v) { return v.first == key; });
        if (itr != end(data)) {
            return itr->second;
        } else {
            throw std::range_error("Not Found");
        }
    }
};

#endif /* ifndef LINGLONG_BOX_SRC_UTIL_CONSTEXPR_MAP */
