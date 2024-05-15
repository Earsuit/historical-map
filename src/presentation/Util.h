#ifndef SRC_PRESENTATION_UTIL_H
#define SRC_PRESENTATION_UTIL_H

#include "src/persistence/Data.h"

#include <string>
#include <memory>

namespace presentation {
struct Color {
    float red;
    float green;
    float blue;
    float alpha;

    auto operator<=>(const Color& other) const noexcept = default;
};
}

#endif
