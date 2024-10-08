SET(UI_SRC
    HistoricalMap.cpp
    HistoricalMap.h
    MapWidget.cpp
    MapWidget.h
    TileSourceWidget.cpp
    TileSourceWidget.h
    LogWidget.cpp
    LogWidget.h
    DefaultInfoWidget.cpp
    DefaultInfoWidget.h
    IInfoWidget.h
    Util.cpp
    Util.h
    ExportInfoWidget.h
    ExportInfoWidget.cpp
    ImportInfoWidget.h
    ImportInfoWidget.cpp
    MapWidgetNoninteractive.h
    MapWidgetNoninteractive.cpp
    TileSourceUrlWidget.h
    TileSourceUrlWidget.cpp
)

add_library(libui STATIC ${UI_SRC} ${IMGUI_SRC} ${IMPLOT_SRC} ${IM_FILE_DIALOG_SRC})
target_link_libraries(libui PRIVATE OpenGL::GL glfw GLEW::GLEW ${CMAKE_DL_LIBS} liblogger Intl::Intl libpersistence)
target_include_directories(libui PUBLIC ${GLEW_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS})

if (LINUX)
    target_link_libraries(libui PRIVATE PkgConfig::GIO2)
endif()