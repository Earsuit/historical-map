#include "src/presentation/LogPresenter.h"

#include <libintl.h>

namespace presentation {
LogPresenter::LogPresenter(LogWidgetInterface& view):
    view{view},
    model{model::LogModel::getInstance()},
    logger{logger::LoggerManager::getInstance().getLogger("LogPresenter")}
{
}

void LogPresenter::handleSetFilter(const std::string& pattern)
{
    try {
        regex = std::regex{pattern};
        filterEnable = true;
    } catch (const std::regex_error& e) {
        logger.error("{}", gettext("Invalid regex string: ") + std::string(e.what()));
        filterEnable = false;
    }
}

void LogPresenter::handleDisplayLogs()
{
    model::LogModel::Log log;

    while (model.getQueue().try_dequeue(log)) {
        logs[end++] = std::move(log);

        if (end == start) {
            start++;
        }
    }

    for (auto i = start; i != end; i++) {
        if (filterEnable && !std::regex_search(logs[i].msg, regex)) {
            continue;
        }

        view.displayLog(ImVec4{logs[i].color.r, logs[i].color.g, logs[i].color.b, logs[i].color.a}, logs[i].msg);
    }
}

void LogPresenter::handleSetLevel(int idx)
{
    model.setLevel(spdlog::level::level_enum{idx});
}

std::array<std::string, NUM_LEVLES> LogPresenter::handleGetLevels() const
{
    return std::array<std::string, NUM_LEVLES>{"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRITICAL", "OFF"};
}
}