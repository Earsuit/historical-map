SET(UI_SRC
    HistoricalMap.cpp
    MapPlot.cpp
    TileSourceWidget.cpp
    util.cpp
)

target_sources(HistoricalMap PRIVATE ${UI_SRC})