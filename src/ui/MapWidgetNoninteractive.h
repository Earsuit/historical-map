#ifndef SRC_UI_MAP_WIDGET_INTERACTIVE_H
#define SRC_UI_MAP_WIDGET_INTERACTIVE_H

#include "src/ui/MapWidget.h"

namespace ui {
class MapWidgetNoninteractive: public MapWidget {
public:
    MapWidgetNoninteractive(const std::string& source):
        MapWidget{source}
    {}

private:
    size_t dragPointId = 0;

    virtual void renderRightClickMenu() override {};
    virtual bool renderPoint(ImVec2& coordinate, float size, const ImVec4& color) override;
};
}

#endif
