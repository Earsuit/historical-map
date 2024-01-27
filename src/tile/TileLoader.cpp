#include "src/tile/TileLoader.h"
#include "src/logger/Util.h"

#include <chrono>
#include <cmath>

namespace tile {
constexpr int TILE_CACHE_DEPTH = 1;

using namespace std::chrono_literals;

TileLoader::TileLoader():
    logger{spdlog::get(logger::LOGGER_NAME)}
{
}

void TileLoader::request(const Coordinate& coord)
{
    if (!tileSource) {
        logger->debug("TileLoader doesn't have a valid TileSource object, abort loading tiles.");
        return;
    }

    if (!tileEngine) {
        logger->debug("Tile data processor doesn't have a valid object, abort loading tiles.");
        return;
    }

    if (!(futureData.contains(coord) || tiles[coord.z].contains(coord))) {
        logger->debug("Request tile at x={}, y={}, z={}", coord.x, coord.y, coord.z);
        futureData.emplace(
            std::make_pair(coord, std::async(std::launch::async, [coord, 
                                                                  tileSource = this->tileSource,
                                                                  tileEngine = this->tileEngine](){
                    return tileEngine->toImage(tileSource->request(coord));
                }))
        );
    }
}

void TileLoader::load(const Coordinate& coord)
{
    if (futureData.contains(coord) && futureData[coord].wait_for(0s) == std::future_status::ready) {
        tiles[coord.z].emplace(std::make_pair(coord, std::make_shared<Tile>(coord, futureData[coord].get())));
        logger->debug("Tile at x={}, y={}, z={} is ready", coord.x, coord.y, coord.z);

        futureData.erase(coord);
    }
}

std::optional<std::shared_ptr<Tile>> TileLoader::loadTile(const Coordinate& coord)
{
    request(coord);

    load(coord);

    if (tiles[coord.z].contains(coord)) {
        return tiles[coord.z][coord];
    }

    resourceClean(coord);
    
    return std::nullopt;
}

void TileLoader::resourceClean(const Coordinate& coord)
{
    for (int i = MIN_ZOOM_LEVEL; i < MAX_ZOOM_LEVEL; i++) {
        if (std::abs(i - coord.z) > TILE_CACHE_DEPTH && !tiles[i].empty()) {
            logger->debug("Clear cache for zoom {}, current zoom {}", i, coord.z);
            tiles[i].clear();
        }
    }
}

void TileLoader::setTileSource(std::shared_ptr<TileSource> tileSource)
{
    if (this->tileSource != tileSource) {
        clearCache();

        this->tileSource = tileSource;
    }
}

void TileLoader::setTileEngine(std::shared_ptr<TileEngine> tileEngine)
{
    if (this->tileEngine != tileEngine) {
        clearCache();

        this->tileEngine = tileEngine;
    }
}

void TileLoader::clearCache()
{
    tiles = std::array<std::map<Coordinate, std::shared_ptr<Tile>>, MAX_ZOOM_LEVEL>{};
    futureData.clear();
}

}