SET(PRESENTATION_SRC
    MapWidgetPresenter.h
    MapWidgetPresenter.cpp
)

add_library(libpresentation STATIC ${PRESENTATION_SRC})
target_link_libraries(libpresentation PRIVATE libtile)