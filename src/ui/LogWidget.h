#ifndef SRC_UI_LOG_WIDGET_H
#define SRC_UI_LOG_WIDGET_H

#include "src/logger/LoggerManager.h"
#include "src/logger/LogWidgetInterface.h"
#include "src/logger/Util.h"

#include "imgui.h"
#include "concurrentqueue.h"

#include <array>
#include <string>
#include <optional>

namespace ui {

constexpr uint8_t BIT_NUM = 9;
// The real logs we can store is 2^n -1 due to start == end is treat as empty
constexpr int MAX_SIZE = (1 << BIT_NUM);
constexpr auto LOG_WIDGET_NAME = "Log";

class LogWidget : public logger::LogWidgetInterface {
public:
    LogWidget();
    void paint();

    void log(const std::string& log, spdlog::level::level_enum lvl) override;

private:
    struct Log {
        std::string msg;
        std::optional<ImVec4> color;
    };

    int logLevel = spdlog::level::info;
    logger::LoggerManager& loggerManager;
    moodycamel::ConcurrentQueue<Log> queue;
    std::array<Log, MAX_SIZE> logs;
    logger::Index<uint16_t, BIT_NUM> start{0};
    logger::Index<uint16_t, BIT_NUM> end{0};
    std::string filter;
    bool filterEnable = false;

    void updateLogs();
};
}

#endif
