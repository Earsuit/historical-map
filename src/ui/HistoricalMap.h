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

namespace ui {

class HistoricalMap {
public:
    HistoricalMap();
    ~HistoricalMap();

    void start();

private:
    GLFWwindow* window;
    LogWidget logWidget;
    std::shared_ptr<IInfoWidget> infoWidget;
    MapWidget mapWidget;
    TileSourceWidget tileSourceWidget;

    void buildDockSpace(ImGuiIO& io);
};

}

#endif
