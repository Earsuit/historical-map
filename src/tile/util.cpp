#include "src/tile/util.h"

#include <cmath>

namespace tile {

constexpr float PI_DEGREE = 360.0;
constexpr float HALF_PI_DEGREE = 180.0;

std::tuple<int, int, int> lonLat2Cords(float latitude, float longitude, int zoom)
{
    const auto n = 1 << zoom;

    const auto x = static_cast<int>(std::floor(n * (longitude + HALF_PI_DEGREE) / PI_DEGREE));

    // to radians
    latitude = latitude * M_PI / HALF_PI_DEGREE;
    const auto y = static_cast<int>(std::floor(n * (1 - (std::asinh(std::tan(latitude)) / M_PI)) / 2));

    return {x, y, n};
}

std::tuple<float, float> cords2LonLat(int x, int y, int zoom)
{
    const auto n = 1 << zoom;

    const float longitude = PI_DEGREE * x / n - HALF_PI_DEGREE;
    const float latitude = std::atan(std::sinh(M_PI * (1- 2*y/n))) * HALF_PI_DEGREE / M_PI;

    return {longitude, latitude};
}

}