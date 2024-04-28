#ifndef SRC_PRESENTATION_UTIL_H
#define SRC_PRESENTATION_UTIL_H

namespace presentation {
struct Vec2 {
    float x;
    float y;

    auto operator<=>(const Vec2& other) const noexcept = default;
};

struct Range {
    float min;
    float max;

    auto operator<=>(const Range& other) const noexcept = default;
};

// definition https://wiki.openstreetmap.org/wiki/Bounding_Box
struct BoundingBox {
    float west = 0.0f;
    float south = 0.0f;
    float east = 0.0f;
    float north = 0.0f;
};
}

#endif
