#ifndef SRC_UI_MAP_WIDGET_INTERACTIVE_H
#define SRC_UI_MAP_WIDGET_INTERACTIVE_H

#include "src/ui/MapWidget.h"

namespace ui {
class MapWidgetNoninteractive: public MapWidget {
public:
    MapWidgetNoninteractive(const std::string& source):
        MapWidget{source}
    {}

    virtual model::Vec2 renderPoint(const model::Vec2& coordinate, float size, const presentation::Color& color) override;

private:
    size_t dragPointId = 0;

    virtual void renderRightClickMenu() override {};

    virtual void prepareRenderPoint() override;
};
}

#endif
