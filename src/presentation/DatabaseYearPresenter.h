#ifndef SRC_PRESENTATION_DATABASE_YEAR_PRESENTER_H
#define SRC_PRESENTATION_DATABASE_YEAR_PRESENTER_H

#include "src/model/DatabaseModel.h"
#include "src/model/DynamicInfoModel.h"

#include "blockingconcurrentqueue.h"
#include "spdlog/spdlog.h"

#include <thread>
#include <atomic>

namespace presentation {
class DatabaseYearPresenter {
public:
    DatabaseYearPresenter();
    virtual ~DatabaseYearPresenter();

    void handleMoveYearForward() noexcept;
    void handleMoveYearBackward() noexcept;
    void handleSetYear(int year) noexcept;
    int handelGetYear() const noexcept;
    int handleGetMaxYear() const noexcept { return getMaxYear(); }
    int handleGetMinYear() const noexcept { return getMinYear(); }

private:
    std::shared_ptr<spdlog::logger> logger;
    model::DatabaseModel& databaseModel;
    model::DynamicInfoModel& dynamicInfoModel;
    moodycamel::BlockingConcurrentQueue<std::function<void()>> taskQueue;
    std::atomic_bool runWorkerThread;
    std::thread workerThread;

    void updateInfo();
    void worker();
    void startWorkerThread();
    void stopWorkerThread();

    virtual int moveYearForward() noexcept;
    virtual int moveYearBackward() noexcept;
    virtual int setYear(int year) noexcept;
    virtual int getMaxYear() const noexcept { return databaseModel.getMaxYear(); }
    virtual int getMinYear() const noexcept { return databaseModel.getMinYear(); }
};
}

#endif
