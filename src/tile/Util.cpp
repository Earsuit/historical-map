#include "src/tile/Util.h"

#include <cmath>

namespace tile {

constexpr int TILE_SIZE = 256;
constexpr float PI_DEG = 360.0;
constexpr float HALF_PI_DEG = 180.0;

std::tuple<int, int, int> lonLat2Cords(float latitude, float longitude, int zoom)
{
    const auto n = 1 << zoom;

    const auto x = static_cast<int>(std::floor(n * (longitude + HALF_PI_DEG) / PI_DEG));

    // to radians
    latitude = latitude * M_PI / HALF_PI_DEG;
    const auto y = static_cast<int>(std::floor(n * (1 - (std::asinh(std::tan(latitude)) / M_PI)) / 2));

    return {x, y, n};
}

std::tuple<float, float> cords2LonLat(int x, int y, int zoom)
{
    const auto n = 1 << zoom;

    const float longitude = PI_DEG * x / n - HALF_PI_DEG;
    const float latitude = std::atan(std::sinh(M_PI * (1- 2*y/n))) * HALF_PI_DEG / M_PI;

    return {longitude, latitude};
}

float deg2Rad(float degree)
{
    return degree * M_PI / HALF_PI_DEG;
}

float rad2Deg(float radius)
{
    return radius * HALF_PI_DEG / M_PI;
}

// https://learn.microsoft.com/en-us/azure/azure-maps/zoom-levels-and-tile-grid?tabs=csharp#tile-math-source-code
int bestZoomLevel(const BoundingBox& bbox, int padding, int mapWidth, int mapHeight)
{
    const float longitudeDelta = bbox.east > bbox.west ? 
                                 bbox.east - bbox.west : 
                                 PI_DEG - (bbox.west - bbox.east);
    const float resolutionHorizontal = longitudeDelta / (mapWidth - padding * 2);

    const float ry1 = std::log((std::sin(deg2Rad(bbox.south)) + 1) / std::cos(deg2Rad(bbox.south)));
    const float ry2 = std::log((std::sin(deg2Rad(bbox.north)) + 1) / std::cos(deg2Rad(bbox.north)));
    const float centerLat = rad2Deg(std::atan(std::sinh((ry1 + ry2) / 2)));
    const float vy0 = std::log(tanf(M_PI * (0.25f + centerLat / PI_DEG)));
    const float vy1 = std::log(tanf(M_PI * (0.25f + bbox.north / PI_DEG)));
    const float zoomFactorPowered = (mapHeight * 0.5f - padding) / (40.7436654315252 * (vy1 - vy0));
    const float resolutionVertical = PI_DEG / (zoomFactorPowered * TILE_SIZE);

    const float resolution = std::max(resolutionVertical, resolutionHorizontal);

    return static_cast<int>(std::log2(PI_DEG / (resolution * PI_DEG)));
}

}