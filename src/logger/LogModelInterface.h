#ifndef SRC_LOGGER_LOG_MODEL_INTERFACE_H
#define SRC_LOGGER_LOG_MODEL_INTERFACE_H

#include "spdlog/spdlog.h"

#include <string>

namespace logger {
class LogModelInterface {
public:
    virtual void addLog(const std::string& log, spdlog::level::level_enum lvl) = 0;
};
}

#endif /* SRC_LOGGER_LOG_MODEL_INTERFACE_H */
