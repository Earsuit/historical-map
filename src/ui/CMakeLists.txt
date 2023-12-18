SET(UI_SRC
    HistoricalMap.cpp
    MapWidget.cpp
    TileSourceWidget.cpp
    Util.cpp
    LogWidget.cpp
)

target_sources(HistoricalMap PRIVATE ${UI_SRC})