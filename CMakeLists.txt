cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
project(Historical_map C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(external/imgui)
include_directories(external/implot)
include_directories(external/stb)
include_directories(external/spdlog/include)

SET(IMGUI_SRC 
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/misc/cpp/imgui_stdlib.cpp
)

SET(IMPLOT_SRC
    external/implot/implot.cpp
    external/implot/implot_items.cpp
)

add_executable(HistoricalMap main.cpp ${IMGUI_SRC} ${IMPLOT_SRC} ${STB_SRC})
target_link_libraries(HistoricalMap PRIVATE OpenGL::GL glfw GLEW::GLEW Threads::Threads CURL::libcurl ${CMAKE_DL_LIBS})

add_subdirectory(src)

