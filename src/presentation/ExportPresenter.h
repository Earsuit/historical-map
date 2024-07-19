#ifndef SRC_PRESENTATION_EXPORT_PRESENTER_H
#define SRC_PRESENTATION_EXPORT_PRESENTER_H

#include "src/model/ExportModel.h"
#include "src/model/CacheModel.h"
#include "src/logger/ModuleLogger.h"

#include <string>
#include <vector>
#include <future>
#include <atomic>

namespace presentation {
class ExportPresenter {
public:
    ExportPresenter(const std::string& source);

    std::vector<std::string> handleRequestSupportedFormat() const { return exportModel.getSupportedFormat(); }
    util::Expected<void> handleSetFormat(const std::string& format) { return exportModel.setFormat(format); }
    void handleDoExport(const std::string& file);
    util::Expected<bool> handleCheckExportComplete();
    float handleRequestExportProgress() const noexcept;

private:
    logger::ModuleLogger logger;
    model::CacheModel& dynamicModel;
    model::ExportModel exportModel;
    std::string source;
    std::future<util::Expected<void>> task;
    std::atomic_int total;
    std::atomic_int progress;
};
}

#endif
