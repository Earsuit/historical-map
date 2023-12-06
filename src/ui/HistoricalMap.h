#ifndef SRC_UI_HISTORICAL_MAP_H
#define SRC_UI_HISTORICAL_MAP_H

#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_glfw.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace ui {

class HistoricalMap {
public:
    HistoricalMap();
    ~HistoricalMap();

    void start();

private:
    GLFWwindow* window;
    ImVec4 clear_color;
};

}

#endif
