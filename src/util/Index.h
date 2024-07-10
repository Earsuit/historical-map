#ifndef SRC_UTIL_INDEX_H
#define SRC_UTIL_INDEX_H

#include <cstdint>
#include <type_traits>
#include <climits>
#include <compare>

namespace util {
template<typename Y, uint8_t BIT_NUM>
requires(std::is_unsigned_v<Y> && sizeof(Y) * CHAR_BIT > BIT_NUM)
struct Index {
    Y idx: BIT_NUM;

    template<typename T>
    requires(std::is_integral_v<T>)
    Index& operator=(T val) noexcept
    {
        idx = val;
        return *this;
    }

    Index& operator=(const Index& other) noexcept
    {
        if (this == &other) {
            return *this;
        }

        idx = other.idx;
        return *this;
    }

    auto operator<=>(const Index& other) const noexcept = default;

    Index operator+(const Index& other) const noexcept
    {
        return {idx + other.idx};
    }

    Index& operator++() noexcept
    {
        idx++;

        return *this;
    }

    Index operator++(int) noexcept
    {
        Index old = *this;
        operator++();
        return old;
    }

    template<typename T>
    requires(std::is_integral_v<T>)
    void operator+=(T val) noexcept
    {
        idx += val;
    }

    operator Y() const noexcept
    {
        return idx;
    }
};

}

#endif
