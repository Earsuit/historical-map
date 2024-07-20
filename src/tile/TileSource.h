#ifndef SRC_TILE_TILE_SOURCE_H
#define SRC_TILE_TILE_SOURCE_H

#include "Tile.h"
#include "src/tile/Util.h"

#include <vector>
#include <memory>

namespace tile {

class TileSource {
public:
    virtual ~TileSource() = default;

    virtual std::vector<std::byte> request(const Coordinate& coord) = 0;
    virtual void stopAllRequests() = 0;
};

}

#endif
