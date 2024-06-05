#ifndef SRC_PRESENTATION_DATABASE_SAVER_PRESENTER_H
#define SRC_PRESENTATION_DATABASE_SAVER_PRESENTER_H

#include "src/model/CacheModel.h"
#include "src/model/DatabaseModel.h"
#include "src/logger/ModuleLogger.h"

#include "blockingconcurrentqueue.h"

#include <atomic>
#include <string>
#include <thread>

namespace presentation {
class DatabaseSaverPresenter {
public:
    DatabaseSaverPresenter(const std::string& source);
    ~DatabaseSaverPresenter() { stopWorkerThread(); }

    bool handleSaveSameForRange(int startYear, int endYear);
    void handleSaveAll();
    bool isSaveComplete() const noexcept { return saveComplete; }
    float getProgress() const noexcept;

private:
    logger::ModuleLogger logger;
    model::DatabaseModel& databaseModel;
    model::CacheModel& cacheModel;
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
