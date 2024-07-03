set(MODEL_SRC
    DatabaseModel.h
    DatabaseModel.cpp
    CacheModel.h
    CacheModel.cpp
    TileModel.h
    TileModel.cpp
    Util.h
    Util.cpp
    ExportModel.h
    ExportModel.cpp
    ImportModel.h
    ImportModel.cpp
    LogModel.h
    LogModel.cpp
)

add_library(libmodel STATIC ${MODEL_SRC})
# libpersistence have to be made as public because the presenter needs the header of sqlite3.h
# this is only a problem on Windows compilation
target_link_libraries(libmodel PRIVATE libtile liblogger PUBLIC libpersistence)
target_include_directories(libmodel PUBLIC ${SQLite3_INCLUDE_DIRS})