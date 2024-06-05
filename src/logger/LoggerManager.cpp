#include "src/logger/LoggerManager.h"

namespace logger {
const auto PATTERN = "[%D %T %z] [%n] [%l] %v";

LoggerManager& LoggerManager::getInstance(std::vector<spdlog::sink_ptr> sinks)
{
    static LoggerManager manager{sinks};
    return manager;
}

ModuleLogger LoggerManager::getLogger(const std::string& name)
{
    std::shared_ptr<spdlog::logger> logger;
    {
        std::shared_lock lk{lock};
        if (!loggers.contains(name)) {
            logger = loggers[name].lock();
        }
    }
    
    if (!logger) {
        logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
        logger->set_pattern(PATTERN);
        logger->set_level(level);

        {
            std::unique_lock lk{lock};
            loggers[name] = logger;
        }
    }

    return ModuleLogger{logger};
}

void LoggerManager::setLevel(spdlog::level::level_enum lvl)
{
    std::unique_lock lk{lock};
    level = lvl;
    for (auto& [name, ptr] : loggers) {
        if (auto logger = ptr.lock(); logger) {
            logger->set_level(level);
        }
    }
}
}