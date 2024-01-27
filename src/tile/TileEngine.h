#ifndef SRC_TILE_TILEENGINE
#define SRC_TILE_TILEENGINE

#include <vector>
#include <tuple>
#include <cstddef>

namespace tile {
struct TileEngine {
    using RgbBlob = std::vector<std::byte>;
    using Width = int;
    using Height = int;
    using Channels = int;
    using Image = std::tuple<RgbBlob, Width, Height, Channels>;

    virtual ~TileEngine() = default;

    virtual Image toImage(const std::vector<std::byte>& rawBlob) = 0;
};
}

#endif /* SRC_TILE_TILEENGINE */
