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
elseif(MSVC)
# silence compile warning from spdlog
add_definitions(-D_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)

if (UNIX)
    find_package(PkgConfig REQUIRED)
endif()

if (LINUX)
    pkg_check_modules(GIO2 REQUIRED IMPORTED_TARGET gio-2.0)
endif()

if (MSVC)
    # set no char8_t (back to C++17)
    # enable utf-8 support
    # disable program warning from mapbox point.hpp: warning c4068 unknown pragma 'gcc'
    add_compile_options(/Zc:char8_t- /utf-8 -wd4068)
else()
    # -ftemplate-depth for the DEFAULT_FILE_ICON and DEFAULT_FOLDER_ICON
    add_compile_options(-fno-char8_t -ftemplate-depth=2048)
    if (APPLE)
        # https://github.com/llvm/llvm-project/issues/48757, use -Wno-elaborated-enum-base to suppress the enum bug on Apple
        # and also suppress some other warnings from Apple whcih we can't fix now
        add_compile_options(-Wno-elaborated-enum-base 
                            -ObjC++ 
                            -DGL_SILENCE_DEPRECATION 
                            -Wno-deprecated-declarations
                            -Wno-deprecated-anon-enum-enum-conversion
                            -Wno-missing-declarations)
    endif()
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
target_link_libraries(HistoricalMap PRIVATE glfw GLEW::GLEW libui libpresentation  -Wl,-force_load libpersistence liblogger)
target_link_libraries(HistoricalMap PRIVATE "-framework CoreFoundation" "-framework CoreGraphics" "-framework ImageIO" "-framework AppKit")
elseif(LINUX)
target_link_libraries(HistoricalMap PRIVATE libui libpresentation -Wl,--whole-archive libpersistence -Wl,--no-whole-archive liblogger)
elseif(MSVC)
target_link_libraries(HistoricalMap PRIVATE libui libpresentation liblogger libpersistence)
set_property(TARGET HistoricalMap APPEND_STRING PROPERTY LINK_FLAGS " /WHOLEARCHIVE:libpersistence")
endif()

add_subdirectory(src)
add_subdirectory(test)

add_custom_command(TARGET HistoricalMap POST_BUILD
    COMMAND 
    ${CMAKE_COMMAND} 
    -E copy_if_different 
    ${CMAKE_SOURCE_DIR}/fonts/LXGWNeoXiHei.ttf  
    $<TARGET_FILE_DIR:HistoricalMap>    # use TARGET_FILE_DIR to ensure the font is copied to the directory with the executable, such as build/Debug
)