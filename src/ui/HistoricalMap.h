#ifndef SRC_UI_HISTORICAL_MAP_H
#define SRC_UI_HISTORICAL_MAP_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "src/ui/TileSourceWidget.h"

namespace ui {

class HistoricalMap {
public:
    HistoricalMap();
    ~HistoricalMap();

    void start();

private:
    GLFWwindow* window;
    TileSourceWidget tileSourceWidget;
};

}

#endif
