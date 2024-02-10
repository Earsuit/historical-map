#ifndef SRC_PERSISTENCE_PERSISTENCE_MANAGER_H
#define SRC_PERSISTENCE_PERSISTENCE_MANAGER_H

#include "src/persistence/Persistence.h"
#include "src/persistence/DataCache.h"

#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"
#include "readerwriterqueue.h"
#include "spdlog/spdlog.h"

#include <future>
#include <set>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <optional>

namespace persistence {
class PersistenceManager {
public:
    PersistenceManager();
    ~PersistenceManager();

    std::shared_ptr<Data> load(int year);
    void remove(const std::shared_ptr<Data> data);
    void update(const std::shared_ptr<Data> data);

    size_t getWorkLoad();

private:
    Persistence<sqlpp::sqlite3::connection, sqlpp::sqlite3::connection_config> persistence;
    moodycamel::BlockingReaderWriterQueue<std::function<void()>> taskQueue;
    moodycamel::BlockingReaderWriterQueue<Data> loadQueue;
    std::shared_ptr<spdlog::logger> logger;
    DataCache<Data> cache;
    std::set<int> requested;
    std::atomic_bool runWorkerThread;
    std::thread workerThread;

    bool request(int year);
    void worker();
    void startWorkerThread();
    void stopWorkerThread();
};
}

#endif
