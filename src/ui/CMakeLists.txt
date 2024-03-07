SET(UI_SRC
    HistoricalMap.cpp
    MapWidget.cpp
    TileSourceWidget.cpp
    LogWidget.cpp
    CountryInfoWidget.cpp
    HistoricalInfoWidget.cpp
    IInfoWidget.cpp
    Util.cpp
)

add_library(libui STATIC ${UI_SRC} ${IMGUI_SRC} ${IMPLOT_SRC})
target_link_libraries(libui PRIVATE OpenGL::GL glfw GLEW::GLEW ${CMAKE_DL_LIBS})