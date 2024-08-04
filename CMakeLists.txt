cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

project(Historical_map CXX)

set(CMAKE_CXX_STANDARD 20 ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD_REQUIRED ON ON CACHE INTERNAL "")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(LOCALE "zh_CN.UTF-8")
set(LOCALE_DIR "${CMAKE_SOURCE_DIR}/locale")
set(SINGLE_PO_FILE "${CMAKE_BINARY_DIR}/HistoricalInfo.po")
set(POT_FILE "${LOCALE_DIR}/translation.pot")
set(PO_FILE "${LOCALE_DIR}/${LOCALE}/translation.po")
set(ICON_FILE ${CMAKE_SOURCE_DIR}/assets/AppIcon.png)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

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
find_package(GLEW REQUIRED MODULE)  # have to use module mode then the GLEW_INCLUDE_DIRS can be set 
find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Intl REQUIRED)
find_package(Gettext REQUIRED)
find_package(GTest REQUIRED)
include(GoogleTest)
enable_testing()
find_package(Sqlpp11 REQUIRED)
set(BUILD_SQLITE3_CONNECTOR ON)

find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext REQUIRED)
find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge REQUIRED)
find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt REQUIRED)
find_program(GETTEXT_MSGINIT_EXECUTABLE msginit REQUIRED)

include(GeneratePoFile)
include(TagVersion)
include(GetArchitecture)
createTagVersion()
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK})
getArchitecture(ARCHITECTURE)
message("Host architecture is ${ARCHITECTURE}")

set(EXECUTABLE_NAME "HistoricalMap")

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
include_directories(${Intl_INCLUDE_DIRS})

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
add_compile_definitions(USE_GETTEXT)

if (APPLE)
    add_executable(HistoricalMap MACOSX_BUNDLE main.cpp ${ICON_FILE})
    # set_target_properties(HistoricalMap PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
    # On macos we have to specify glfw GLEW::GLEW when compiling main.cpp because it includes HistoricalMap.h
    # which includes glfw and glew
    target_link_libraries(HistoricalMap PRIVATE glfw GLEW::GLEW libui libpresentation  -Wl,-force_load libpersistence liblogger)
    target_link_libraries(HistoricalMap PRIVATE "-framework CoreFoundation" "-framework CoreGraphics" "-framework ImageIO" "-framework AppKit")
    
    set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(HistoricalMap PROPERTIES
        OUTPUT_NAME ${EXECUTABLE_NAME}
        MACOSX_BUNDLE TRUE
        MACOSX_RPATH TRUE
        MACOSX_BUNDLE_BUNDLE_NAME ${EXECUTABLE_NAME}
        MACOSX_BUNDLE_ICON_FILE AppIcon
        MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        MACOSX_BUNDLE_GUI_IDENTIFIER Earsuit.${EXECUTABLE_NAME}
        MACOSX_FRAMEWORK_IDENTIFIER Earsuit.${EXECUTABLE_NAME}
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME TRUE
        XCODE_ATTRIBUTE_EXECUTABLE_NAME ${EXECUTABLE_NAME}
    )

    # build dmg
    install(TARGETS HistoricalMap BUNDLE DESTINATION .)
    set(CPACK_GENERATOR DragNDrop)
elseif(LINUX)
    add_executable(HistoricalMap main.cpp)
    target_link_libraries(HistoricalMap PRIVATE libui libpresentation -Wl,--whole-archive libpersistence -Wl,--no-whole-archive liblogger)

    set(INSTALL_DIR "/opt/${EXECUTABLE_NAME}")
    configure_file(${CMAKE_SOURCE_DIR}/Linux/HistoricalMap.desktop.in HistoricalMap.desktop @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/script/DebianPostinst.sh.in postinst @ONLY)
    configure_file(${CMAKE_SOURCE_DIR}/script/DebianPrerm.sh.in prerm @ONLY)
    execute_process(COMMAND chmod +x ${CMAKE_BINARY_DIR}/postinst)
    execute_process(COMMAND chmod +x ${CMAKE_BINARY_DIR}/prerm)
    install(TARGETS HistoricalMap RUNTIME DESTINATION ${INSTALL_DIR})
    install(DIRECTORY $<TARGET_FILE_DIR:HistoricalMap>/locale DESTINATION ${INSTALL_DIR})
    install(FILES $<TARGET_FILE_DIR:HistoricalMap>/LXGWNeoXiHei.ttf DESTINATION ${INSTALL_DIR})
    install(FILES $<TARGET_FILE_DIR:HistoricalMap>/HistoricalMap.desktop DESTINATION /usr/share/applications)
    install(FILES ${CMAKE_SOURCE_DIR}/assets/AppIcon.png DESTINATION ${INSTALL_DIR})
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_BINARY_DIR}/postinst ${CMAKE_BINARY_DIR}/prerm)
    set(CPACK_PACKAGE_CONTACT "renyudicn@outlook.com")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libglu1-mesa-dev")
    set(CPACK_GENERATOR DEB)
elseif(MSVC)
    add_executable(HistoricalMap WIN32 main.cpp ${CMAKE_SOURCE_DIR}/Windows/resource.rc)
    target_link_libraries(HistoricalMap PRIVATE libui libpresentation liblogger libpersistence)
    set_property(TARGET HistoricalMap APPEND_STRING PROPERTY LINK_FLAGS "/WHOLEARCHIVE:libpersistence")

    add_custom_command(TARGET HistoricalMap POST_BUILD
        COMMAND 
        ${CMAKE_COMMAND} 
        -E copy_if_different 
        ${CMAKE_SOURCE_DIR}/Windows/ca-bundle.crt
        ${CMAKE_SOURCE_DIR}/Windows/HistoricalMap.exe.manifest
        $<TARGET_FILE_DIR:HistoricalMap>    # use TARGET_FILE_DIR to ensure the font is copied to the directory with the executable, such as build/Debug
    )

    install(TARGETS HistoricalMap RUNTIME DESTINATION .)
    install(DIRECTORY $<TARGET_FILE_DIR:HistoricalMap>/locale DESTINATION .)
    install(FILES 
            $<TARGET_FILE_DIR:HistoricalMap>/HistoricalMap.lib 
            $<TARGET_FILE_DIR:HistoricalMap>/HistoricalMap.exp 
            $<TARGET_FILE_DIR:HistoricalMap>/LXGWNeoXiHei.ttf 
            $<TARGET_FILE_DIR:HistoricalMap>/ca-bundle.crt
            DESTINATION .)
    set(CPACK_GENERATOR ZIP)
endif()

set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME "${EXECUTABLE_NAME}-${VERSION}-${ARCHITECTURE}")
include(CPack)

add_subdirectory(src)
add_subdirectory(test)

file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*" ${IM_FILE_DIALOG_SRC})
generatePoFile("${SOURCES}")

add_custom_command(TARGET HistoricalMap POST_BUILD
    COMMAND 
    ${CMAKE_COMMAND} 
    -E copy_if_different 
    ${CMAKE_SOURCE_DIR}/fonts/LXGWNeoXiHei.ttf  
    $<TARGET_FILE_DIR:HistoricalMap>    # use TARGET_FILE_DIR to ensure the font is copied to the directory with the executable, such as build/Debug
)

set(TRANS_DIR $<TARGET_FILE_DIR:HistoricalMap>/locale/zh_CN/LC_MESSAGES/)
add_custom_command(
    TARGET HistoricalMap POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TRANS_DIR}
    COMMENT "Creating directory: ${TRANS_DIR}"
)

# translation po -> mo file
# It is up to the programmer to decide whether to update the mo file
# because translation is a manual work
add_custom_command(
    OUTPUT DUMMY
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} ${PO_FILE} -o ${TRANS_DIR}/HistoricalInfo.mo 
    COMMENT "Compile the po file"
)
add_custom_target(translation DEPENDS DUMMY)