#ifndef SRC_TILE_UTIL_H
#define SRC_TILE_UTIL_H

#include <tuple>

namespace tile {

std::tuple<int, int, int> lonLat2Cords(float latitude, float longitude, int zoom);
std::tuple<float, float> cords2LonLat(int x, int y, int zoom);

}

#endif
