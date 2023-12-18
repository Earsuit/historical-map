#ifndef SRC_UI_LOG_WIDGET_H
#define SRC_UI_LOG_WIDGET_H

#include "src/logger/StringSink.h"

#include "spdlog/spdlog.h"

#include <vector>
#include <string>

namespace ui {
class LogWidget {
public:
    LogWidget(spdlog::logger& logger, std::shared_ptr<logger::StringSink> sink): 
        sink{sink},
        logger{logger} 
    {}
    void paint();

private:
    std::shared_ptr<logger::StringSink> sink;
    spdlog::logger& logger;
    std::vector<std::string> logs;
};
}

#endif
