#include "src/tile/TileLoader.h"

namespace tile {
TileLoader::TileLoader(spdlog::logger& logger):
    logger{logger}
{
}

void setTileSource(std::shared_ptr<TileSource> tileSource)
{
    tileSource = tileSource;
}

void TileLoader::load(int xMin, int xMax, int yMin, int yMax, int zoom)
{
    for (auto x = xMin; x <= xMax; x++) {
        for (auto y = yMin; y <= yMax; y++) {
            if (!tiles.contains({x, y, zoom})) {
                logger.debug("Request tile at x={}, y={}, z={}", x, y, zoom);
                tileSource->request(x, y, zoom);
            }
        }
    }

    for (auto tile : tileSource->takeReady()) {
        const auto coord = tile->getCoordinate();
        logger.debug("Tile at x={}, y={}, z={} is ready", coord.x, coord.y, coord.z);
        tiles.emplace(std::make_pair(coord, tile));
    }
}

std::optional<std::shared_ptr<Tile>> TileLoader::getTile(int x, int y, int zoom)
{
    if (tiles.contains({x, y, zoom})) {
        return tiles[{x, y, zoom}];
    }
    
    return std::nullopt;
}

}