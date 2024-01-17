SET(UI_SRC
    HistoricalMap.cpp
    MapWidget.cpp
    TileSourceWidget.cpp
    LogWidget.cpp
    CountryInfoWidget.cpp
    HistoricalInfoWidget.cpp
)

target_sources(HistoricalMap PRIVATE ${UI_SRC})