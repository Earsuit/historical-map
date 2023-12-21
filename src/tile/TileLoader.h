#ifndef SRC_TILE_TILE_LOADER_H
#define SRC_TILE_TILE_LOADER_H

#include "src/tile/TileSource.h"
#include "src/tile/Tile.h"
#include "src/tile/Util.h"

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

    std::optional<std::shared_ptr<Tile>> loadTile(const Coordinate& coord);

private:
    spdlog::logger& logger;
    std::shared_ptr<TileSource> tileSource;
    std::map<Coordinate, std::shared_ptr<Tile>> tiles;
    std::map<Coordinate, std::future<std::shared_ptr<Tile>>> futureTiles;

    void request(const Coordinate& coord);
    void load(const Coordinate& coord);
};
}

#endif
