#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "external/imgui/imgui.h"

#include <utility>

namespace ui {

class MapPlot {
public:
    void paint();

private:
    std::pair<ImVec2, ImVec2> calculateBound(int x, int y);
};
}

#endif
