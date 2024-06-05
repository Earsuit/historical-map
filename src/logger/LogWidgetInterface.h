#ifndef SRC_LOGGER_LOG_WIDGET_INTERFACE_H
#define SRC_LOGGER_LOG_WIDGET_INTERFACE_H

#include "spdlog/spdlog.h"

#include <string>

namespace logger {
class LogWidgetInterface {
public:
    virtual void log(const std::string& log, spdlog::level::level_enum lvl) = 0;
};
}

#endif
