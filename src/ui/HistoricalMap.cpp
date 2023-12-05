#include "src/ui/HistoricalMap.h"

#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_glfw.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>

namespace ui {

namespace {
static void glfw_error_callback(int error, const char* description)
{
    const std::string message = "Glfw Error " + std::to_string(error) + ": " + description;
    throw std::runtime_error(message);
}
}

bool HistoricalMap::prepare()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return false;
    }
    
    
}

}