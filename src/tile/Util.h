#ifndef SRC_TILE_UTIL_H
#define SRC_TILE_UTIL_H

#include <tuple>

namespace tile {

// definition https://wiki.openstreetmap.org/wiki/Bounding_Box
struct BoundingBox {
    float west;
    float south;
    float east;
    float north;
};

float deg2Rad(float degree);
float rad2Deg(float radius);
std::tuple<int, int, int> lonLat2Cords(float latitude, float longitude, int zoom);
std::tuple<float, float> cords2LonLat(float x, float y, int zoom);
int bestZoomLevel(const BoundingBox& bbox, int padding, int mapWidth, int mapHeight);

}

#endif
