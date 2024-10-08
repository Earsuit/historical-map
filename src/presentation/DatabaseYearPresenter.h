#ifndef SRC_PRESENTATION_DATABASE_YEAR_PRESENTER_H
#define SRC_PRESENTATION_DATABASE_YEAR_PRESENTER_H

#include "src/model/DatabaseModel.h"
#include "src/model/CacheModel.h"
#include "src/util/Worker.h"
#include "src/util/Signal.h"
#include "src/logger/ModuleLogger.h"

#include "blockingconcurrentqueue.h"

#include <thread>
#include <atomic>

namespace presentation {
class DatabaseYearPresenter {
public:
    DatabaseYearPresenter();
    ~DatabaseYearPresenter();

    void handleMoveYearForward() noexcept;
    void handleMoveYearBackward() noexcept;
    void handleSetYear(int year) noexcept;
    int handelGetYear() const noexcept { return databaseModel.getYear(); }
    int handleGetMaxYear() const noexcept { return databaseModel.getMaxYear(); }
    int handleGetMinYear() const noexcept { return databaseModel.getMinYear(); }

    util::signal::Signal<void(int)> onYearChange;

private:
    logger::ModuleLogger logger;
    model::DatabaseModel& databaseModel;
    model::CacheModel& cacheModel;
    util::Worker<std::function<void()>> worker;

    void updateInfo(int year);
};
}

#endif
