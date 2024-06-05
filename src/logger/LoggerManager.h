#ifndef SRC_LOGGER_LOGGER_MANAGER_H
#define SRC_LOGGER_LOGGER_MANAGER_H

#include "src/logger/ModuleLogger.h"

#include "spdlog/spdlog.h"

#include <memory>
#include <map>
#include <shared_mutex>

namespace logger {
class LoggerManager {
public:
    // the call with sink setup has to be first, otherwise you can't setup the sinks
    static LoggerManager& getInstance(std::vector<spdlog::sink_ptr> sinks = {});

    ModuleLogger getLogger(const std::string& name);
    void setLevel(spdlog::level::level_enum level);

    LoggerManager(LoggerManager&&) = delete;
    LoggerManager(const LoggerManager&) = delete;
    LoggerManager& operator=(const LoggerManager&) = delete;

private:
    std::shared_mutex lock;
    std::vector<spdlog::sink_ptr> sinks;
    std::map<std::string, std::weak_ptr<spdlog::logger>> loggers;
    spdlog::level::level_enum level = spdlog::level::info;

    LoggerManager(const std::vector<spdlog::sink_ptr>& sinks):
        sinks{sinks}
    {}
};
}

#endif
