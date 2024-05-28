#ifndef SRC_PRESENTATION_IMPORT_YEAR_PRESENTER_H
#define SRC_PRESENTATION_IMPORT_YEAR_PRESENTER_H

#include "src/model/DatabaseModel.h"
#include "src/model/DynamicInfoModel.h"
#include "src/util/Worker.h"

#include "spdlog/spdlog.h"

#include <string>
#include <memory>
#include <set>

namespace presentation {
class ImportYearPresenter {
public:
    ImportYearPresenter(const std::string& source);

    void initYearsList();

    void handleMoveYearForward() noexcept;
    void handleMoveYearBackward() noexcept;
    void handleSetYear(int year) noexcept;
    int handelGetYear() const noexcept;
    int handleGetMaxYear() const noexcept;
    int handleGetMinYear() const noexcept;

private:
    std::shared_ptr<spdlog::logger> logger;
    model::DatabaseModel& databaseModel;
    model::DynamicInfoModel& dynamicInfoModel;
    std::string source;
    std::set<int> years;
    util::Worker<std::function<void()>> worker;

    void updateInfo();
};
}

#endif
