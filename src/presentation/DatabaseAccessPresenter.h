#ifndef SRC_PRESENTATION_DATABASE_INOUT_PRESENTER_H
#define SRC_PRESENTATION_DATABASE_INOUT_PRESENTER_H

#include "src/model/DynamicInfoModel.h"
#include "src/model/DatabaseModel.h"

#include "spdlog/spdlog.h"
#include "blockingconcurrentqueue.h"

#include <atomic>
#include <string>
#include <thread>
#include <atomic>

namespace presentation {
class DatabaseAccessPresenter {
public:
    DatabaseAccessPresenter(const std::string& source);
    ~DatabaseAccessPresenter() { stopWorkerThread(); }

    bool handleSave(int startYear, int endYear);
    bool isSaveComplete() const noexcept { return saveComplete; }
    float getProgress() const noexcept;
    void handleRefresh();

private:
    std::shared_ptr<spdlog::logger> logger;
    model::DatabaseModel& databaseModel;
    model::DynamicInfoModel& dynamicInfoModel;
    std::string source;
    moodycamel::BlockingConcurrentQueue<std::function<void()>> taskQueue;
    std::atomic_int progress;
    int total;
    std::atomic_bool runWorkerThread;
    std::thread workerThread;
    std::atomic_bool saveComplete;

    void worker();
    void startWorkerThread();
    void stopWorkerThread();
};
}

#endif