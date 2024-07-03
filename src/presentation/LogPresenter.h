#ifndef SRC_PRESENTATION_LOG_PRESENTER_H
#define SRC_PRESENTATION_LOG_PRESENTER_H

#include "src/util/Index.h"
#include "src/logger/ModuleLogger.h"
#include "src/model/LogModel.h"
#include "src/presentation/LogWidgetInterface.h"

#include <string>
#include <regex>
#include <array>

namespace presentation {
constexpr int NUM_LEVLES = 7;

class LogPresenter {
public:
    LogPresenter(LogWidgetInterface& view);    

    void handleSetFilter(const std::string& pattern);
    void handleUnsetFilter() noexcept { filterEnable = false; }
    bool handleCheckIsFilterSet() const noexcept { return filterEnable; };
    void handleClearLogs() noexcept { start = end = 0; }
    void handleDisplayLogs();
    void handleSetLevel(int idx);
    int handleGetLevel() const noexcept { return model.getLevel(); }
    std::array<std::string, NUM_LEVLES> handleGetLevels() const;

private:
    static constexpr uint8_t BIT_NUM = 9;
    // The real logs we can store is 2^n -1 due to start == end is treat as empty
    static constexpr int MAX_SIZE = (1 << BIT_NUM);

    LogWidgetInterface& view;
    model::LogModel& model;
    logger::ModuleLogger logger;
    bool filterEnable = false;
    std::regex regex;
    std::array<model::LogModel::Log, MAX_SIZE> logs;
    util::Index<uint16_t, BIT_NUM> start{0};
    util::Index<uint16_t, BIT_NUM> end{0};
};
}

#endif /* SRC_PRESENTATION_LOG_PRESENTER_H */
