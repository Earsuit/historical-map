#ifndef SRC_TILE_TILE_LOADER_H
#define SRC_TILE_TILE_LOADER_H

#include "src/tile/TileSource.h"
#include "src/tile/Tile.h"
#include "src/tile/Util.h"
#include "src/tile/TileEngine.h"

#include "spdlog/spdlog.h"

#include <memory>
#include <optional>
#include <map>
#include <tuple>
#include <cstddef>
#include <vector>
#include <array>
#include <future>

namespace tile {
class TileLoader {
public:
    TileLoader();

    void setTileSource(std::shared_ptr<TileSource> tileSource);
    void setTileEngine(std::shared_ptr<TileEngine> tileDataProcessor);

    std::optional<std::shared_ptr<Tile>> loadTile(const Coordinate& coord);

private:
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<TileSource> tileSource;
    std::shared_ptr<TileEngine> tileEngine;
    std::array<std::map<Coordinate, std::shared_ptr<Tile>>, MAX_ZOOM_LEVEL> tiles;
    std::map<Coordinate, std::future<std::optional<tile::TileEngine::Image>>> futureData;

    void request(const Coordinate& coord);
    void load(const Coordinate& coord);
    void resourceClean(const Coordinate& coord);
    void clearCache();
};
}

#endif
