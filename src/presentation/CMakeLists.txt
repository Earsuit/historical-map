SET(PRESENTATION_SRC
    Util.h
    MapWidgetPresenter.h
    MapWidgetPresenter.cpp
    MapWidgetInterface.h
    DatabaseYearPresenter.h
    DatabaseYearPresenter.cpp
    HistoricalInfoPresenter.h
    HistoricalInfoPresenter.cpp
    DatabaseAccessPresenter.h
    DatabaseAccessPresenter.cpp
)

add_library(libpresentation STATIC ${PRESENTATION_SRC})
target_link_libraries(libpresentation PRIVATE libtile libmodel)