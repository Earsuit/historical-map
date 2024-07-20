#ifndef SRC_TILE_TILE_LOADER_H
#define SRC_TILE_TILE_LOADER_H

#include "src/tile/TileSource.h"
#include "src/tile/Tile.h"
#include "src/tile/Util.h"
#include "src/tile/TileEngine.h"
#include "src/util/Cache.h"
#include "src/logger/ModuleLogger.h"

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
    static TileLoader& getInstance();

    void setTileSource(std::shared_ptr<TileSource> tileSource);
    void setTileEngine(std::shared_ptr<TileEngine> tileDataProcessor);

    std::optional<std::shared_ptr<Tile>> loadTile(const Coordinate& coord);
    void clearCache();

private:
    TileLoader();

    logger::ModuleLogger logger;
    std::shared_ptr<TileSource> tileSource;
    std::shared_ptr<TileEngine> tileEngine;
    util::Cache<Coordinate, std::shared_ptr<Tile>> cache;
    std::map<Coordinate, std::future<std::optional<tile::TileEngine::Image>>> futureData;

    void request(const Coordinate& coord);
    void load(const Coordinate& coord);
};
}

#endif
