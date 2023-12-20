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

struct Coordinate {
    int x = 0;
    int y = 0;
    int z = 0;

    auto operator<=>(const Coordinate& other) const noexcept = default;
};

float deg2Rad(float degree);
float rad2Deg(float radius);
int longitude2X(float longitude, int zoom);
int latitude2Y(float latitude, int zoom);
float x2Longitude(float x, int zoom);
float y2Latitude(float y, int zoom);
int bestZoomLevel(const BoundingBox& bbox, int padding, int mapWidth, int mapHeight);

}

#endif
