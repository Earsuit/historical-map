#ifndef SRC_PRESENTATION_IMPORT_PRESENTER_H
#define SRC_PRESENTATION_IMPORT_PRESENTER_H

#include "src/model/CacheModel.h"
#include "src/model/ImportModel.h"
#include "src/util/Error.h"
#include "src/logger/ModuleLogger.h"

#include "tl/expected.hpp"

#include <string>
#include <vector>
#include <future>
#include <atomic>

namespace presentation {
class ImportPresenter {
public:
    ImportPresenter(const std::string& source);
    ~ImportPresenter();

    void handleDoImport(const std::string& file);
    tl::expected<bool, util::Error> handleCheckImportComplete();
    auto handleGetImportedYears() const { return cacheModel.getYearList(source); }
    std::vector<std::string> handleGetSupportedFormat() const { return importModel.getSupportedFormat(); }
    void handleCancelImport() { stopImport = true; }

private:
    logger::ModuleLogger logger;
    model::CacheModel& cacheModel;
    model::ImportModel importModel;
    std::string source;
    std::future<tl::expected<void, util::Error>> task;
    std::atomic_bool stopImport = false;
};
}

#endif
