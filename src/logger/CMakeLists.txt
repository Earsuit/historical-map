SET(LOGGER_SRC
    ModelSink.cpp
    ModelSink.h
    LoggerManager.h
    LoggerManager.cpp
    LogModelInterface.h
)

add_library(liblogger STATIC ${LOGGER_SRC})