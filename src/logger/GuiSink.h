#ifndef SRC_LOGGER_GUI_SINK_H
#define SRC_LOGGER_GUI_SINK_H

#include "src/logger/LogWidgetInterface.h"

#include "spdlog/sinks/base_sink.h"

namespace logger {
class GuiSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    GuiSink(LogWidgetInterface& view):
        view{view}
    {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;

    void flush_() override {};

private:
    LogWidgetInterface& view;
};
} // namespace logger


#endif
