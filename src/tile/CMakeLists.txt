SET(TILE_SRC
    Tile.cpp
    Util.cpp
    TileSourceUrl.cpp
    TileLoader.cpp
    RasterTileEngine.cpp
    TileEngineFactory.cpp
)

add_library(libtile STATIC ${TILE_SRC})
target_link_libraries(libtile PRIVATE CURL::libcurl)