SET(UI_SRC
    HistoricalMap.cpp
    MapWidget.cpp
    TileSourceWidget.cpp
    LogWidget.cpp
    CountryInfoWidget.cpp
    HistoricalInfoWidget.cpp
    IInfoWidget.cpp
    Util.cpp
    ExportWidget.cpp
)

add_library(libui STATIC ${UI_SRC} ${IMGUI_SRC} ${IMPLOT_SRC} ${IM_FILE_DIALOG_SRC})
target_link_libraries(libui PRIVATE OpenGL::GL glfw GLEW::GLEW ${CMAKE_DL_LIBS} PkgConfig::GIO2 PkgConfig::GTK3 libstbImageImpl)