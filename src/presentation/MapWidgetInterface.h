#ifndef SRC_PRESENTATION_MAP_WIDGET_INTERFACE_H
#define SRC_PRESENTATION_MAP_WIDGET_INTERFACE_H

#include "src/model/Util.h"
#include "src/presentation/Util.h"

#include <string>
#include <optional>
#include <vector>

namespace presentation {
class MapWidgetInterface {
public:
    virtual model::Range getAxisRangeX() const noexcept = 0;
    virtual model::Range getAxisRangeY() const noexcept = 0;
    virtual model::Vec2 getPlotSize() const noexcept = 0;
    virtual void renderTile(void* texture, 
                            const model::Vec2& bMin, 
                            const model::Vec2& bMax) = 0;
    virtual std::optional<model::Vec2> getMousePos() const = 0;
};
}

#endif
