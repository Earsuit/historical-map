#ifndef SRC_LOGGER_MODULE_LOGGER_H
#define SRC_LOGGER_MODULE_LOGGER_H

#include "spdlog/spdlog.h"

#include <memory>

namespace logger {
class ModuleLogger {
public:
    ModuleLogger(std::shared_ptr<spdlog::logger> logger):
        logger{logger}
    {}

    template<typename... Args>
    void trace(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        logger->log(spdlog::level::trace, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void debug(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        logger->log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        logger->log(spdlog::level::info, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        logger->log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        logger->log(spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void critical(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        logger->log(spdlog::level::critical, fmt, std::forward<Args>(args)...);
    }

private:
    std::shared_ptr<spdlog::logger> logger;
};
}

#endif
