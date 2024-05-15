set(MODEL_SRC
    DatabaseModel.h
    DatabaseModel.cpp
    DynamicInfoModel.h
    DynamicInfoModel.cpp
    TileModel.h
    TileModel.cpp
    Util.h
    Util.cpp
)

add_library(libmodel STATIC ${MODEL_SRC})
target_link_libraries(libmodel PRIVATE libpersistence)