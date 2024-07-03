#include "src/model/LogModel.h"
#include "src/logger/ModelSink.h"

namespace model {
constexpr auto ERROR_COLLOR = Color{1.0f, 0.4f, 0.4f, 1.0f};
constexpr auto DEFAULT_COLLOR = Color{0.0f, 0.0f, 0.0f, 1.0f};

LogModel& LogModel::getInstance()
{
    static LogModel model;
    return model;
}

LogModel::LogModel():
    loggerManager{logger::LoggerManager::getInstance({std::make_shared<logger::ModelSink>(*this)})}
{
}

void LogModel::addLog(const std::string& log, spdlog::level::level_enum lvl)
{
    Color color = DEFAULT_COLLOR;

    if (lvl >= spdlog::level::level_enum::err) {
        color = ERROR_COLLOR;
    }

    queue.enqueue({log, color});
}

void LogModel::setLevel(spdlog::level::level_enum level)
{
    loggerManager.setLevel(level);
}
}