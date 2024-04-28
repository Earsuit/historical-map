#ifndef SRC_TILE_UTIL_H
#define SRC_TILE_UTIL_H

#include <tuple>

namespace tile {
struct Coordinate {
    int x = 0;
    int y = 0;
    int z = 0;

    auto operator<=>(const Coordinate& other) const noexcept = default;
};
}

#endif
