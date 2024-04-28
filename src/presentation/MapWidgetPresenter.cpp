#include "src/presentation/MapWidgetPresenter.h"

#include <algorithm>

namespace presentation {
constexpr int TILE_SIZE = 256;  
constexpr int BBOX_ZOOM_LEVEL = 0;
constexpr int MIN_ZOOM_LEVEL = 0;
constexpr int MAX_ZOOM_LEVEL = 18;
constexpr float PI_DEG = 360.0;
constexpr float HALF_PI_DEG = 180.0;
constexpr int PADDING = 0;
constexpr float MAX_LONGITUDE = 180.0f;
constexpr float MIN_LONGITUDE = -180.0f;
constexpr float MAX_LATITUDE = 85.05112878f;
constexpr float MIN_LATITUDE = -85.05112878f;
constexpr float PI = M_PI;

std::vector<std::shared_ptr<tile::Tile>> MapWidgetPresenter::getTiles(const Range& xAxis,
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

    zoom = std::clamp(bestZoomLevel(bbox, plotSize.x, plotSize.y), MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL);

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

float MapWidgetPresenter::longitude2X(float longitude) const noexcept
{
    return longitude2X(longitude, BBOX_ZOOM_LEVEL);
}

float MapWidgetPresenter::latitude2Y(float latitude) const
{
    return latitude2Y(latitude, BBOX_ZOOM_LEVEL);
}

float MapWidgetPresenter::longitude2X(float longitude, int zoom) const noexcept
{
    const auto n = 1 << zoom;

    return n * (longitude + HALF_PI_DEG) / PI_DEG;
}

float MapWidgetPresenter::latitude2Y(float latitude, int zoom) const
{
    const auto n = 1 << zoom;

    latitude = latitude * PI / HALF_PI_DEG;
    return n * (1 - (std::asinh(std::tan(latitude)) / PI)) / 2;
}

float MapWidgetPresenter::x2Longitude(float x) const noexcept
{
    return x2Longitude(x, BBOX_ZOOM_LEVEL);
}

float MapWidgetPresenter::x2Longitude(float x, int zoom) const noexcept
{
    const auto n = 1 << zoom;

    return std::clamp(PI_DEG * x / n - HALF_PI_DEG, MIN_LONGITUDE, MAX_LONGITUDE);
}

float MapWidgetPresenter::y2Latitude(float y) const
{
    return y2Latitude(y, BBOX_ZOOM_LEVEL);
}

float MapWidgetPresenter::y2Latitude(float y, int zoom) const
{
    const auto n = 1 << zoom;

    return std::clamp(std::atanf(std::sinh(PI * (1- 2*y/n))) * HALF_PI_DEG / PI,
                      MIN_LATITUDE,
                      MAX_LATITUDE);
}

float MapWidgetPresenter::deg2Rad(float degree) const noexcept
{
    return degree * PI / HALF_PI_DEG;
}

float MapWidgetPresenter::rad2Deg(float radius) const noexcept
{
    return radius * HALF_PI_DEG / PI;
}

float MapWidgetPresenter::computeTileBound(int coord) const noexcept
{
    return static_cast<float>(coord) / (1<<zoom);
}

int MapWidgetPresenter::bestZoomLevel(const BoundingBox& bbox, int mapWidth, int mapHeight) const
{
    const float longitudeDelta = bbox.east > bbox.west ? 
                                 bbox.east - bbox.west : 
                                 PI_DEG - (bbox.west - bbox.east);
    const float resolutionHorizontal = longitudeDelta / (mapWidth - PADDING * 2);

    const float ry1 = std::log((std::sin(deg2Rad(bbox.south)) + 1) / std::cos(deg2Rad(bbox.south)));
    const float ry2 = std::log((std::sin(deg2Rad(bbox.north)) + 1) / std::cos(deg2Rad(bbox.north)));
    const float centerLat = rad2Deg(std::atan(std::sinh((ry1 + ry2) / 2)));
    const float vy0 = std::log(tanf(PI * (0.25f + centerLat / PI_DEG)));
    const float vy1 = std::log(tanf(PI * (0.25f + bbox.north / PI_DEG)));
    const float zoomFactorPowered = (mapHeight * 0.5f - PADDING) / (40.7436654315252 * (vy1 - vy0));
    const float resolutionVertical = PI_DEG / (zoomFactorPowered * TILE_SIZE);

    const float resolution = std::max(resolutionVertical, resolutionHorizontal);

    return static_cast<int>(std::log2(PI_DEG / (resolution * TILE_SIZE)));
}
}