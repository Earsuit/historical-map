#ifndef SRC_TILE_TILE_SOURCE_H
#define SRC_TILE_TILE_SOURCE_H

#include "Tile.h"

#include <vector>
#include <memory>

namespace tile {

class TileSource {
public:
    virtual ~TileSource() = default;

    virtual void request(int x, int y, int z) = 0;

    virtual void waitAll() = 0;

    virtual bool isAllReady() = 0;

    virtual void takeReady(std::vector<std::shared_ptr<Tile>>& tiles) = 0;
};

}

#endif
