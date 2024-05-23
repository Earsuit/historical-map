#include "src/tile/TileLoader.h"
#include "src/logger/Util.h"

#include <chrono>
#include <cmath>

namespace tile {
constexpr int TILE_CACHE_SIZE = 256;

using namespace std::chrono_literals;

TileLoader::TileLoader():
    logger{spdlog::get(logger::LOGGER_NAME)},
    cache{TILE_CACHE_SIZE}
{
}

TileLoader& TileLoader::getInstance()
{
    static TileLoader loader;
    return loader;
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

    if (!(futureData.contains(coord) || cache.contains(coord))) {
        logger->debug("Request tile at x={}, y={}, z={}", coord.x, coord.y, coord.z);
        futureData.emplace(
            std::make_pair(coord, std::async(std::launch::async, [coord, 
                                                                  tileSource = this->tileSource,
                                                                  tileEngine = this->tileEngine]() -> std::optional<tile::TileEngine::Image>
                {
                    if (const auto& data = tileSource->request(coord); data.empty()) {
                        return std::nullopt;
                    } else {
                        return tileEngine->toImage(data);
                    }
                }))
        );
    }
}

void TileLoader::load(const Coordinate& coord)
{
    if (futureData.contains(coord) && futureData[coord].wait_for(0s) == std::future_status::ready) {
        if (auto&& image = futureData[coord].get(); image) {
            cache[coord] = std::make_shared<Tile>(coord, std::move(*image));
            logger->debug("Tile at x={}, y={}, z={} is ready", coord.x, coord.y, coord.z);
        } else {
            logger->debug("Tile at x={}, y={}, z={} failed to load.", coord.x, coord.y, coord.z);
        }

        futureData.erase(coord);
    }
}

std::optional<std::shared_ptr<Tile>> TileLoader::loadTile(const Coordinate& coord)
{
    request(coord);

    load(coord);

    if (cache.contains(coord)) {
        return cache[coord];
    }
    
    return std::nullopt;
}

void TileLoader::setTileSource(std::shared_ptr<TileSource> tileSource)
{
    clearCache();

    this->tileSource = tileSource;
}

void TileLoader::setTileEngine(std::shared_ptr<TileEngine> tileEngine)
{
    clearCache();

    this->tileEngine = tileEngine;
}

void TileLoader::clearCache()
{
    cache.reset();
    futureData.clear();
}

}