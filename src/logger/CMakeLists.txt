SET(LOGGER_SRC
    GuiSink.cpp
    GuiSink.h
    LoggerManager.h
    LoggerManager.cpp
    LogWidgetInterface.h
    Util.h
)

add_library(liblogger STATIC ${LOGGER_SRC})