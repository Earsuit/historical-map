cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
project(Historical_map C CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(external/imgui)

SET(IMGUI_SRC 
    external/imgui/imgui.h
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_internal.h
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imstb_rectpack.h
    external/imgui/imstb_textedit.h
    external/imgui/imstb_truetype.h
    external/imgui/backends/imgui_impl_glfw.h
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.h
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(HistoricalMap main.cpp ${IMGUI_SRC})
target_link_libraries(HistoricalMap PRIVATE OpenGL::GL glfw GLEW::GLEW ${CMAKE_DL_LIBS})

add_subdirectory(src)

