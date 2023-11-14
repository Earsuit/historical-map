#ifndef SRC_TILESOURCE
#define SRC_TILESOURCE

#include "Tile.h"

#include <vector>
#include <memory>

class TileSource {
public:
    virtual ~TileSource() = default;

    virtual bool request(int x, int y, int z) = 0;

    virtual void waitAll() = 0;

    virtual void isAllReady() = 0;

    virtual void takeReady(std::vector<std::shared_ptr<Tile>>& tiles) = 0;
};

#endif /* SRC_TILESOURCE */
