#include "src/tile/TileLoader.h"
#include "src/logger/Util.h"

#include <chrono>

namespace tile {
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

    if (!(futureTiles.contains(coord) || tiles.contains(coord))) {
        logger->debug("Request tile at x={}, y={}, z={}", coord.x, coord.y, coord.z);
        futureTiles.emplace(std::make_pair(coord, tileSource->request(coord)));
    }
}

void TileLoader::load(const Coordinate& coord)
{
    if (futureTiles.contains(coord) && futureTiles[coord].wait_for(0s) == std::future_status::ready) {
        this->logger->debug("Tile at x={}, y={}, z={} is ready", coord.x, coord.y, coord.z);
        this->tiles.emplace(std::make_pair(coord, futureTiles[coord].get()));
        futureTiles.erase(coord);
    }
}

std::optional<std::shared_ptr<Tile>> TileLoader::loadTile(const Coordinate& coord)
{
    request(coord);

    load(coord);

    if (tiles.contains(coord)) {
        return tiles[coord];
    }
    
    return std::nullopt;
}

void TileLoader::setTileSource(std::shared_ptr<TileSource> tileSource)
{
    this->tileSource = tileSource;
}

}