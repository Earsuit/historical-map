cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(Historical_map CXX)

set(CMAKE_CXX_STANDARD 20 ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD_REQUIRED ON ON CACHE INTERNAL "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if (APPLE)
# suppress the warning of OpenGL API deprecated since MacOS 10.14
# add std::result_of back to c++20 for mapbox/variant, 
# but it still works on Linux with g++11 with C++20?
add_definitions(-DGL_SILENCE_DEPRECATION -D_LIBCPP_ENABLE_CXX20_REMOVED_TYPE_TRAITS)
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

if (UNIX)
    find_package(PkgConfig REQUIRED)
endif()

if (LINUX)
    pkg_check_modules(GIO2 REQUIRED IMPORTED_TARGET gio-2.0)
    pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
endif()

if (MSVC)
    add_compile_options(/Zc:char8_t-)
else()
    # -ftemplate-depth for the DEFAULT_FILE_ICON and DEFAULT_FOLDER_ICON
    add_compile_options(-fno-char8_t -ftemplate-depth=2048)
endif()

include(FetchContent)

# sqlpp11
FetchContent_Declare(sqlpp11
    GIT_REPOSITORY  https://github.com/rbock/sqlpp11
    GIT_TAG         0.64
)
set(BUILD_SQLITE3_CONNECTOR ON)
FetchContent_MakeAvailable(sqlpp11)

# gtest
if (GTEST_ENABLE)
cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()
include(GoogleTest)
include_directories(${googletest_SOURCE_DIR}/googletest/include)
endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(external/imgui)
include_directories(external/implot)
include_directories(external/stb)
include_directories(external/spdlog/include)
include_directories(external/cereal/include)
include_directories(external/concurrentqueue)
include_directories(external/variant/include)
include_directories(external/geometry/include)
include_directories(external/polylabel/include)
include_directories(external/json/single_include)
include_directories(external/expected/include)
include_directories(external/imFileDialog)
include_directories(external/magic_enum/include)
include_directories(${CMAKE_BINARY_DIR}) # For generated headers
include_directories(${sqlpp11_SOURCE_DIR}/include)
include_directories(${date_SOURCE_DIR}/include) # dependency of sqlpp11

SET(IMGUI_SRC 
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/misc/cpp/imgui_stdlib.cpp
)

SET(IMPLOT_SRC
    ${CMAKE_SOURCE_DIR}/external/implot/implot.cpp
    ${CMAKE_SOURCE_DIR}/external/implot/implot_items.cpp
)

SET(IM_FILE_DIALOG_SRC
    ${CMAKE_SOURCE_DIR}/external/imFileDialog/ImFileDialog.cpp
)

add_executable(HistoricalMap main.cpp)

if (APPLE)
# On macos we have to specify glfw GLEW::GLEW when compiling main.cpp because it includes HistoricalMap.h
# which includes glfw and glew
target_link_libraries(HistoricalMap PRIVATE glfw GLEW::GLEW libui libtile  -Wl,-force_load libpersistence liblogger)
else()
target_link_libraries(HistoricalMap PRIVATE libui libtile -Wl,--whole-archive libpersistence -Wl,--no-whole-archive liblogger)
endif()

add_subdirectory(src)
add_subdirectory(test)