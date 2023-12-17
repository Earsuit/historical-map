#ifndef SRC_LOGGER_STRING_SINK_H
#define SRC_LOGGER_STRING_SINK_H

#include "spdlog/sinks/base_sink.h"

#include <string>
#include <vector>
#include <utility>
#include <mutex>

namespace logger
{
class StringSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    const std::vector<std::string> dumpLogs();

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;

    void flush_() override;

private:
    std::vector<std::string> logs;
};
} // namespace logger


#endif
