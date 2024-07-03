#ifndef SRC_MODEL_LOG_MODEL_H
#define SRC_MODEL_LOG_MODEL_H

#include "src/logger/LoggerManager.h"
#include "src/logger/LogModelInterface.h"
#include "src/model/Util.h"

#include "concurrentqueue.h"

namespace model {
class LogModel: public logger::LogModelInterface {
public:
    struct Log {
        std::string msg;
        Color color;
    };

    static LogModel& getInstance();
    virtual void addLog(const std::string& log, spdlog::level::level_enum lvl) override;
    void setLevel(spdlog::level::level_enum level);
    spdlog::level::level_enum getLevel() const noexcept { return loggerManager.getLevel(); }
    moodycamel::ConcurrentQueue<Log>& getQueue() { return queue; }

private:
    LogModel();

    int logLevel = spdlog::level::info;
    logger::LoggerManager& loggerManager;
    moodycamel::ConcurrentQueue<Log> queue;
};
}

#endif /* SRC_MODEL_LOG_MODEL_H */
