#ifndef SRC_MODEL_TILE_MODEL_H
#define SRC_MODEL_TILE_MODEL_H

#include "src/tile/TileEngineFactory.h"
#include "src/tile/TileLoader.h"
#include "src/model/Util.h"
#include "src/util/Error.h"
#include "src/logger/ModuleLogger.h"

#include <string>
#include <set>
#include <memory>

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

    auto getTileEngineTypes() const noexcept { return tile::TileEngineFactory::getTileEngines(); }
    util::Expected<void> setTileEngine(const std::string& name);
    auto getTileSourceTypes() const noexcept { return supportedSourceType; }
    void setTileSource(std::shared_ptr<tile::TileSource> tileSource);

private:
    TileModel();

    logger::ModuleLogger logger;
    std::set<std::string> supportedSourceType;
    tile::TileLoader& tileLoader;
    int zoom;
    BoundingBox bbox;
};
}

#endif
