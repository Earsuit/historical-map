SET(TILE_SRC
    Tile.cpp
    Tile.h
    Util.h
    Util.cpp
    TileSourceUrl.cpp
    TileSourceUrl.h
)

target_sources(HistoricalMap PRIVATE ${TILE_SRC})