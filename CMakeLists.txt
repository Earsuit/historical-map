cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

project(Historical_map C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

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
include_directories(${CMAKE_BINARY_DIR}) # For generated headers
include_directories(${sqlpp11_SOURCE_DIR}/include)
include_directories(${date_SOURCE_DIR}/include) # dependency of sqlpp11


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
target_link_libraries(HistoricalMap PRIVATE OpenGL::GL glfw GLEW::GLEW Threads::Threads CURL::libcurl sqlpp11::sqlite3 ${CMAKE_DL_LIBS})

add_subdirectory(src)
add_subdirectory(test)