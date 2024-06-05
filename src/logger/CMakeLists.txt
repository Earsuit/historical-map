SET(LOGGER_SRC
    GuiSink.cpp
    GuiSink.h
    LoggerManager.h
    LoggerManager.cpp
    LogWidgetInterface.h
)

add_library(liblogger STATIC ${LOGGER_SRC})