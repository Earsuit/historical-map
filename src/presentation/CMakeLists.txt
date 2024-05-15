SET(PRESENTATION_SRC
    Util.h
    MapWidgetPresenter.h
    MapWidgetPresenter.cpp
    MapWidgetInterface.h
    InfoWidgetPresenter.h
    InfoWidgetPresenter.cpp
    HistoricalInfoWidgetInterface.h
    HistoricalInfoPresenter.h
    HistoricalInfoPresenter.cpp
    DatabaseAccessPresenter.h
    DatabaseAccessPresenter.cpp
)

add_library(libpresentation STATIC ${PRESENTATION_SRC})
target_link_libraries(libpresentation PRIVATE libtile libmodel)