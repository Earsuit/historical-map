#include "src/logger/StringSink.h"

namespace logger {
const std::vector<std::string> StringSink::dumpLogs()
{
    std::lock_guard lock(mutex_);

    return std::exchange(logs, std::vector<std::string>{});
}

void StringSink::sink_it_(const spdlog::details::log_msg& msg)
{
    // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
    // msg.raw contains pre formatted log

    // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
    logs.emplace_back(fmt::to_string(formatted));
}

void StringSink::flush_() 
{
    std::exchange(logs, std::vector<std::string>{});
}

}