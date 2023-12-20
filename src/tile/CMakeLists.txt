SET(TILE_SRC
    Tile.cpp
    Util.cpp
    TileSourceUrl.cpp
    TileLoader.cpp
)

target_sources(HistoricalMap PRIVATE ${TILE_SRC})