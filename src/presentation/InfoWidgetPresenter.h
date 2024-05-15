#ifndef SRC_PRESENTATION_INFO_WIDGET_PRESENTER_H
#define SRC_PRESENTATION_INFO_WIDGET_PRESENTER_H

#include "src/model/DatabaseModel.h"
#include "src/model/DynamicInfoModel.h"

#include "blockingconcurrentqueue.h"
#include "spdlog/spdlog.h"

#include <thread>
#include <atomic>

namespace presentation {
class InfoWidgetPresenter {
public:
    InfoWidgetPresenter();
    ~InfoWidgetPresenter()
    {
        stopWorkerThread();
    }

    void handleMoveYearForward() noexcept;
    void handleMoveYearBackward() noexcept;
    void handleSetYear(int year) noexcept;
    int getYear() const noexcept;
    int getMaxYear() const noexcept { return databaseModel.getMaxYear(); }
    int getMinYear() const noexcept { return databaseModel.getMinYear(); }

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
};
}

#endif
