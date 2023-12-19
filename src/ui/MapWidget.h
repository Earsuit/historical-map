#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "external/imgui/imgui.h"
#include "spdlog/spdlog.h"

#include <utility>

namespace ui {

class MapWidget {
public:
    MapWidget(spdlog::logger& logger): logger{logger} {}
    void paint();

private:
    spdlog::logger& logger;

    std::pair<ImVec2, ImVec2> calculateBound(int x, int y);
};
}

#endif
