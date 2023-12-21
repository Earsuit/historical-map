#ifndef SRC_TILE_TILE_SOURCE_H
#define SRC_TILE_TILE_SOURCE_H

#include "Tile.h"
#include "src/tile/Util.h"

#include <vector>
#include <memory>
#include <future>

namespace tile {

class TileSource {
public:
    virtual ~TileSource() = default;

    virtual std::future<std::vector<std::byte>> request(const Coordinate& coord) = 0;
};

}

#endif
