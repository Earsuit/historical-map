SET(LOGGER_SRC
    StringSink.cpp
    StringSink.h
    Util.h
)

add_library(liblogger STATIC ${LOGGER_SRC})