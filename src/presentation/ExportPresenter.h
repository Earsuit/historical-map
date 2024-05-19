#ifndef SRC_PRESENTATION_EXPORT_PRESENTER_H
#define SRC_PRESENTATION_EXPORT_PRESENTER_H

#include "src/model/ExportModel.h"
#include "src/model/DynamicInfoModel.h"

#include "spdlog/spdlog.h"

#include <memory>
#include <string>
#include <vector>
#include <future>
#include <atomic>

namespace presentation {
class ExportPresenter {
public:
    ExportPresenter(const std::string& source);

    std::vector<std::string> handleRequestSupportedFormat() const { return exportModel.getSupportedFormat(); }
    tl::expected<void, util::Error> handleSetFormat(const std::string& format) { return exportModel.setFormat(format); }
    void handleDoExport(const std::string& file);
    tl::expected<bool, util::Error> handleCheckExportComplete();
    float handleRequestExportProgress() const noexcept;

private:
    std::shared_ptr<spdlog::logger> logger;
    model::DynamicInfoModel& dynamicModel;
    model::ExportModel exportModel;
    std::string source;
    std::future<tl::expected<void, util::Error>> task;
    std::atomic_int total;
    std::atomic_int progress;
};
}

#endif
