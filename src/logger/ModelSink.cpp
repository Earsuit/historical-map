#include "src/logger/ModelSink.h"

namespace logger {
void ModelSink::sink_it_(const spdlog::details::log_msg& msg)
{
    // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
    // msg.raw contains pre formatted log

    // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
    model.addLog(fmt::to_string(formatted), msg.level);
}
}