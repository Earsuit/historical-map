SET(TILE_SRC
    Tile.cpp
    Tile.h
    TileSourceUrl.cpp
    TileSourceUrl.h
    TileLoader.cpp
    TileLoader.h
    RasterTileEngine.cpp
    RasterTileEngine.h
    TileEngineFactory.cpp
    TileEngineFactory.h
    StbImageImpl.cpp
    Util.h
)

add_library(libtile STATIC ${TILE_SRC})
target_link_libraries(libtile PRIVATE CURL::libcurl liblogger)
if(WIN32)
target_link_libraries(libtile PRIVATE winhttp)
endif()
target_include_directories(libtile PRIVATE ${CURL_INCLUDE_DIRS})