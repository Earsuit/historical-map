#include "src/model/TileModel.h"
#include "src/logger/Util.h"

#include <algorithm>

namespace model {
constexpr int MIN_ZOOM_LEVEL = 0;
constexpr int MAX_ZOOM_LEVEL = 18;
constexpr int PADDING = 0;

TileModel& TileModel::getInstance()
{
    static TileModel model;
    return model;
}

TileModel::TileModel():
    logger{spdlog::get(logger::LOGGER_NAME)},
    tileLoader{tile::TileLoader::getInstance()},
    supportedSourceType{"URL"}
{}

std::vector<std::shared_ptr<tile::Tile>> TileModel::getTiles(const Range& xAxis,
                                                             const Range& yAxis,
                                                             const Vec2& plotSize)
{
    std::vector<std::shared_ptr<tile::Tile>> tiles;
    const auto west = x2Longitude(xAxis.min, BBOX_ZOOM_LEVEL);
    const auto east = x2Longitude(xAxis.max, BBOX_ZOOM_LEVEL);
    const auto north = y2Latitude(yAxis.min, BBOX_ZOOM_LEVEL);
    const auto south = y2Latitude(yAxis.max, BBOX_ZOOM_LEVEL);
    bbox = {west, south, east, north};

    logger->trace("west={}, north={}, east={}, south={}", west, north, east, south);

    zoom = std::clamp(bestZoomLevel(bbox, PADDING, plotSize.x, plotSize.y), MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL);

    const auto limit = (1 << zoom) - 1;
    const auto xMin = std::clamp(static_cast<int>(longitude2X(west, zoom)), 0, limit);
    const auto xMax = std::clamp(static_cast<int>(longitude2X(east, zoom)), 0, limit);
    const auto yMin = std::clamp(static_cast<int>(latitude2Y(north, zoom)), 0, limit);
    const auto yMax = std::clamp(static_cast<int>(latitude2Y(south, zoom)), 0, limit);

    logger->trace("Zoom {} tile X from [{}, {}], Y from [{}, {}]", zoom, xMin, xMax, yMin, yMax);

    for (auto x = xMin; x <= xMax; x++) {
        for (auto y = yMin; y <= yMax; y++) {
            if (auto tile = tileLoader.loadTile({x, y, zoom}); tile) {
                tiles.emplace_back(*tile);
            }
        }
    }

    return tiles;
}

Vec2 TileModel::getTileBoundMax(std::shared_ptr<tile::Tile> tile) const noexcept
{
    const auto coord = tile->getCoordinate();

    return {model::computeTileBound(coord.x+1, zoom), 
            model::computeTileBound(coord.y+1, zoom)};
}

Vec2 TileModel::getTileBoundMin(std::shared_ptr<tile::Tile> tile) const noexcept
{
    const auto coord = tile->getCoordinate();

    return {model::computeTileBound(coord.x, zoom), 
            model::computeTileBound(coord.y, zoom)};
}

tl::expected<void, util::Error> TileModel::setTileEngine(const std::string& name)
{
    if (auto engine = tile::TileEngineFactory::createInstance(name); engine) {
        tileLoader.setTileEngine(engine);
        return util::SUCCESS;
    }

    return tl::unexpected{util::Error{util::ErrorCode::INVALID_PARAM, "Invalid tile engine!"}};
}

void TileModel::setTileSource(std::shared_ptr<tile::TileSource> tileSource)
{
    tileLoader.setTileSource(tileSource);
}

}