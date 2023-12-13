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
std::tuple<float, float> cords2LonLat(int x, int y, int zoom);
// https://learn.microsoft.com/en-us/azure/azure-maps/zoom-levels-and-tile-grid?tabs=csharp#tile-math-source-code
int bestZoomLevel(const BoundingBox& bbox, int padding, int tileSize);

}

#endif
