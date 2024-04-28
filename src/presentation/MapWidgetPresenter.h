#ifndef SRC_PRESENTATION_MAP_WIDGET_PRESENTER_H
#define SRC_PRESENTATION_MAP_WIDGET_PRESENTER_H

#include "src/tile/Tile.h"
#include "src/tile/TileLoader.h"
#include "src/logger/Util.h"
#include "src/presentation/Util.h"
#include "src/persistence/Data.h"

#include "spdlog/spdlog.h"

#include <cstddef>
#include <vector>
#include <memory>

namespace presentation {
class MapWidgetPresenter {
public:
    MapWidgetPresenter():
        logger{spdlog::get(logger::LOGGER_NAME)},
        tileLoader{tile::TileLoader::getInstance()}
    {}

    int getZoomLevel() const noexcept { return zoom; }
    BoundingBox getBoundingBox() const noexcept { return bbox; }
    std::vector<std::shared_ptr<tile::Tile>> getTiles(const Range& xAxis,
                                                      const Range& yAxis,
                                                      const Vec2& plotSize);
    float computeTileBound(int coord) const noexcept;
    float x2Longitude(float x) const noexcept;
    float y2Latitude(float y) const;
    float longitude2X(float longitude) const noexcept;
    float latitude2Y(float latitude) const;

private:
    int zoom;
    BoundingBox bbox;
    std::shared_ptr<spdlog::logger> logger;
    tile::TileLoader& tileLoader;

    float deg2Rad(float degree) const noexcept;
    float rad2Deg(float radius) const noexcept;
    int bestZoomLevel(const BoundingBox& bbox, int mapWidth, int mapHeight) const;
    float x2Longitude(float x, int zoom) const noexcept;
    float y2Latitude(float y, int zoom) const;
    float longitude2X(float longitude, int zoom) const noexcept;
    float latitude2Y(float latitude, int zoom) const;
};
}

#endif
