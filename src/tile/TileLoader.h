#ifndef SRC_TILE_TILE_LOADER_H
#define SRC_TILE_TILE_LOADER_H

#include "src/tile/TileSource.h"
#include "src/tile/Tile.h"

#include "spdlog/spdlog.h"

#include <memory>
#include <optional>
#include <map>
#include <tuple>

namespace tile {
class TileLoader {
public:
    TileLoader(spdlog::logger& logger);

    void setTileSource(std::shared_ptr<TileSource> tileSource);

    void load(int xMin, int xMax, int yMin, int yMax, int zoom);
    std::optional<std::shared_ptr<Tile>> getTile(int x, int y, int zoom);

private:
    spdlog::logger& logger;
    std::shared_ptr<TileSource> tileSource;
    std::map<Coordinate, std::shared_ptr<Tile>> tiles;
};
}

#endif
