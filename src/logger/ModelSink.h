#ifndef SRC_LOGGER_MODEL_SINK_H
#define SRC_LOGGER_MODEL_SINK_H

#include "src/logger/LogModelInterface.h"

#include "spdlog/sinks/base_sink.h"

namespace logger {
class ModelSink : public spdlog::sinks::base_sink<std::mutex> {
public:
    ModelSink(LogModelInterface& model):
        model{model}
    {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;

    void flush_() override {};

private:
    LogModelInterface& model;
};
} // namespace logger


#endif /* SRC_LOGGER_MODEL_SINK_H */
