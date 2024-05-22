#ifndef SRC_UI_HISTORICAL_MAP_H
#define SRC_UI_HISTORICAL_MAP_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "src/ui/TileSourceWidget.h"
#include "src/ui/LogWidget.h"
#include "src/ui/MapWidget.h"
#include "src/ui/IInfoWidget.h"
#include "src/logger/StringSink.h"

#include <memory>
#include <vector>

namespace ui {

class HistoricalMap {
public:
    HistoricalMap();
    ~HistoricalMap();

    void start();

private:
    GLFWwindow* window;
    LogWidget logWidget;
    std::unique_ptr<IInfoWidget> infoWidget;
    std::vector<std::unique_ptr<MapWidget>> mapWidgets;
    TileSourceWidget tileSourceWidget;
    int previousDockedMapWidget = 0;
    ImGuiID down, left;

    void buildDockSpace();
    void buildMapDockSpace();
};

}

#endif
