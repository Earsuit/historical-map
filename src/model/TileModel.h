#ifndef SRC_MODEL_TILE_MODEL_H
#define SRC_MODEL_TILE_MODEL_H

#include "src/tile/TileLoader.h"
#include "src/model/Util.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

namespace model {
class TileModel {
public:
    static TileModel& getInstance();

    BoundingBox getBoundingBox() const noexcept { return bbox; }
    std::vector<std::shared_ptr<tile::Tile>> getTiles(const Range& xAxis,
                                                      const Range& yAxis,
                                                      const Vec2& plotSize);
    Vec2 getTileBoundMax(std::shared_ptr<tile::Tile> tile) const noexcept;
    Vec2 getTileBoundMin(std::shared_ptr<tile::Tile> tile) const noexcept;
  
private:
    TileModel():
        logger{spdlog::get(logger::LOGGER_NAME)},
        tileLoader{tile::TileLoader::getInstance()}
    {}

    std::shared_ptr<spdlog::logger> logger;
    tile::TileLoader& tileLoader;
    int zoom;
    BoundingBox bbox;
};
}

#endif
