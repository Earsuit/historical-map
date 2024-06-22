SET(PRESENTATION_SRC
    Util.h
    MapWidgetPresenter.h
    MapWidgetPresenter.cpp
    MapWidgetInterface.h
    DatabaseYearPresenter.h
    DatabaseYearPresenter.cpp
    HistoricalInfoPresenter.h
    HistoricalInfoPresenter.cpp
    DatabaseSaverPresenter.h
    DatabaseSaverPresenter.cpp
    InfoSelectorPresenter.h
    InfoSelectorPresenter.cpp
    ExportPresenter.h
    ExportPresenter.cpp
    ImportPresenter.h
    ImportPresenter.cpp
    ImportYearPresenter.h
    ImportYearPresenter.cpp
    TileSourceWidgetPresenter.h
    TileSourceUrlPresenter.h
    TileSourceUrlPresenter.cpp
    MainViewInterface.h
    MainViewPresenter.h
    MainViewPresenter.cpp
)

add_library(libpresentation STATIC ${PRESENTATION_SRC})
target_link_libraries(libpresentation PRIVATE libtile libmodel liblogger Intl::Intl)
target_include_directories(libpresentation PUBLIC ${SQLite3_INCLUDE_DIRS})

generatePoFile("presentation" ${CMAKE_CURRENT_SOURCE_DIR} "${PRESENTATION_SRC}")