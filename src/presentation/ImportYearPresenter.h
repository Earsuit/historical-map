#ifndef SRC_PRESENTATION_IMPORT_YEAR_PRESENTER_H
#define SRC_PRESENTATION_IMPORT_YEAR_PRESENTER_H

#include "src/model/DatabaseModel.h"
#include "src/model/CacheModel.h"
#include "src/util/Worker.h"
#include "src/util/Signal.h"
#include "src/logger/ModuleLogger.h"

#include <string>
#include <set>

namespace presentation {
class ImportYearPresenter {
public:
    ImportYearPresenter(const std::string& source);
    ~ImportYearPresenter();

    void initYearsList();

    void handleMoveYearForward() noexcept;
    void handleMoveYearBackward() noexcept;
    void handleSetYear(int year) noexcept;
    int handleGetMaxYear() const noexcept;
    int handleGetMinYear() const noexcept;

    util::signal::Signal<void(int)> onYearChange;

private:
    logger::ModuleLogger logger;
    model::DatabaseModel& databaseModel;
    model::CacheModel& cacheModel;
    std::string source;
    std::set<int> years;
    util::Worker<std::function<void()>> worker;

    void updateInfo(int year);
};
}

#endif
