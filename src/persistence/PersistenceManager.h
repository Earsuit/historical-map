#ifndef SRC_PERSISTENCE_PERSISTENCE_MANAGER_H
#define SRC_PERSISTENCE_PERSISTENCE_MANAGER_H

#include "src/persistence/Persistence.h"

#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"
#include "readerwriterqueue.h"
#include "spdlog/spdlog.h"

#include <future>
#include <map>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>

namespace persistence {
class PersistenceManager {
public:
    PersistenceManager();
    ~PersistenceManager();

    Data loadData(int year);
    void remove(const Data data);
    void update(const Data data);

private:
    Persistence<sqlpp::sqlite3::connection_pool, sqlpp::sqlite3::connection_config> persistence;
    moodycamel::BlockingReaderWriterQueue<std::function<void()>> taskQueue;
    moodycamel::BlockingReaderWriterQueue<Data> loadQueue;
    std::shared_ptr<spdlog::logger> logger;
    std::map<int, Data> cache;
    std::atomic_bool runWorkerThread;
    std::thread workerThread;

    void request(int year);
    void worker();
    void startWorkerThread();
    void stopWorkerThread();
};
}

#endif
