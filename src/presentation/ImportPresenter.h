#ifndef SRC_PRESENTATION_IMPORT_PRESENTER_H
#define SRC_PRESENTATION_IMPORT_PRESENTER_H

#include "src/model/DynamicInfoModel.h"
#include "src/model/ImportModel.h"
#include "src/util/Error.h"

#include "tl/expected.hpp"
#include "spdlog/spdlog.h"

#include <string>
#include <vector>
#include <memory>
#include <future>

namespace presentation {
class ImportPresenter {
public:
    ImportPresenter(const std::string& source);

    void handleDoImport(const std::string& file);
    tl::expected<bool, util::Error> handleCheckImportComplete();
    auto handleGetImportedYears() const { return dynamicInfoModel.getYearList(source); }
    std::vector<std::string> handleGetSupportedFormat() const { return importModel.getSupportedFormat(); }

private:
    std::shared_ptr<spdlog::logger> logger;
    model::DynamicInfoModel& dynamicInfoModel;
    model::ImportModel importModel;
    std::string source;
    std::future<tl::expected<void, util::Error>> task;
};
}

#endif
