#ifndef SRC_MODEL_UTIL_H
#define SRC_MODEL_UTIL_H

#include <compare>

namespace model {
constexpr int BBOX_ZOOM_LEVEL = 0;

// definition https://wiki.openstreetmap.org/wiki/Bounding_Box
struct BoundingBox {
    float west = 0.0f;
    float south = 0.0f;
    float east = 0.0f;
    float north = 0.0f;
};

struct Vec2 {
    float x;
    float y;

    auto operator<=>(const Vec2&) const noexcept = default;
};

struct Range {
    float min;
    float max;

    auto operator<=>(const Range&) const noexcept = default;
};

float deg2Rad(float degree);
float rad2Deg(float radius);
float longitude2X(float longitude, int zoom);
float latitude2Y(float latitude, int zoom);
float x2Longitude(float x, int zoom);
float y2Latitude(float y, int zoom);
int bestZoomLevel(const BoundingBox& bbox, int padding, int mapWidth, int mapHeight);
float computeTileBound(int coord, int zoom);
}

#endif
