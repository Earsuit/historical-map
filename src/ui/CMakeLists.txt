SET(UI_SRC
    HistoricalMap.cpp
    MapPlot.cpp
    TileSourceWidget.h
    TileSourceWidget.cpp
    util.cpp
    util.h
)

target_sources(HistoricalMap PRIVATE ${UI_SRC})