#ifndef SRC_UI_LOG_WIDGET_H
#define SRC_UI_LOG_WIDGET_H

#include "src/logger/StringSink.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <array>
#include <string>

namespace ui {

constexpr uint8_t BIT_NUM = 9;
// The real logs we can store is 2^n -1 due to start == end is treat as empty
constexpr int MAX_SIZE = (1 << BIT_NUM);

class LogWidget {
public:
    LogWidget(spdlog::logger& logger, std::shared_ptr<logger::StringSink> sink);
    void paint();

private:
    std::shared_ptr<logger::StringSink> sink;
    spdlog::logger& logger;
    int logLevel = spdlog::level::info;
    std::array<std::string, MAX_SIZE> logs;
    logger::Index<uint16_t, BIT_NUM> start{0};
    logger::Index<uint16_t, BIT_NUM> end{0};

    void updateLogs();
};
}

#endif
