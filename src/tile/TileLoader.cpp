#include "src/tile/TileLoader.h"

#include <chrono>

namespace tile {
using namespace std::chrono_literals;

TileLoader::TileLoader(spdlog::logger& logger):
    logger{logger}
{
}

void TileLoader::load(int xMin, int xMax, int yMin, int yMax, int zoom)
{
    if (!tileSource) {
        logger.debug("TileLoader doesn't have a valid TileSource object, abort loading tiles.");
        return;
    }

    for (auto x = xMin; x <= xMax; x++) {
        for (auto y = yMin; y <= yMax; y++) {
            Coordinate coord{x, y, zoom};
            if (!(futureTiles.contains(coord) || tiles.contains(coord))) {
                logger.debug("Request tile at x={}, y={}, z={}", x, y, zoom);
                futureTiles.emplace(std::make_pair(coord, tileSource->request(coord)));
            }
        }
    }

    std::erase_if(futureTiles, [this](auto& kv){
        auto& [coord, futureTile] = kv;

        if (futureTile.wait_for(0s) == std::future_status::ready) {
            this->logger.debug("Tile at x={}, y={}, z={} is ready", coord.x, coord.y, coord.z);
            this->tiles.emplace(std::make_pair(coord, futureTile.get()));
            return true;
        } else {
            return false;
        }
    });
}

std::optional<std::shared_ptr<Tile>> TileLoader::getTile(int x, int y, int zoom)
{
    if (tiles.contains({x, y, zoom})) {
        return tiles[{x, y, zoom}];
    }
    
    return std::nullopt;
}

void TileLoader::setTileSource(std::shared_ptr<TileSource> tileSource)
{
    this->tileSource = tileSource;
}

}