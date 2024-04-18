#ifndef SRC_PERSISTENCE_DATABASEMANAGER
#define SRC_PERSISTENCE_DATABASEMANAGER

#include "src/persistence/Database.h"
#include "src/util/Cache.h"

#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"
#include "blockingconcurrentqueue.h"
#include "spdlog/spdlog.h"

#include <future>
#include <set>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <optional>

namespace persistence {
class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    ~DatabaseManager();

    std::shared_ptr<Data> load(int year);
    void remove(const std::shared_ptr<const Data> data);
    void update(const std::shared_ptr<const Data> data);

    size_t getWorkLoad();

    DatabaseManager(DatabaseManager&&) = delete;
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    DatabaseManager();

    Database<sqlpp::sqlite3::connection, sqlpp::sqlite3::connection_config> database;
    moodycamel::BlockingConcurrentQueue<std::function<void()>> taskQueue;
    moodycamel::BlockingConcurrentQueue<std::shared_ptr<Data>> loadQueue;
    std::shared_ptr<spdlog::logger> logger;
    util::Cache<int, std::shared_ptr<Data>> cache;
    std::set<int> requesting;
    std::atomic_bool runWorkerThread;
    std::thread workerThread;

    bool request(int year);
    void worker();
    void startWorkerThread();
    void stopWorkerThread();
};
}

#endif /* SRC_PERSISTENCE_DATABASEMANAGER */
