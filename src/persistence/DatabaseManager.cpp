#include "src/persistence/DatabaseManager.h"
#include "src/logger/Util.h"

namespace persistence {
constexpr auto DATABASE_NAME = "HistoricalMapDB";
constexpr auto QUEUE_SIZE = 128;
constexpr auto DATA_CACHE_SIZE = 8;

DatabaseManager::DatabaseManager():
    database{std::make_shared<sqlpp::sqlite3::connection_config>(DATABASE_NAME, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)},
    taskQueue{QUEUE_SIZE},
    loadQueue{QUEUE_SIZE},
    logger{spdlog::get(logger::LOGGER_NAME)},
    cache{DATA_CACHE_SIZE}
{
    startWorkerThread();
}

DatabaseManager::~DatabaseManager()
{
    stopWorkerThread();
}

void DatabaseManager::startWorkerThread()
{
    runWorkerThread = true;
    workerThread = std::thread(&DatabaseManager::worker, this);
}

void DatabaseManager::stopWorkerThread()
{
    runWorkerThread = false;

    // enqueue an empty task to wake up the wait_dequeue if necessary
    taskQueue.enqueue([](){});

    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void DatabaseManager::worker()
{
    while (runWorkerThread) {
        std::function<void()> task; 

        taskQueue.wait_dequeue(task);

        task();
    }
}

std::shared_ptr<Data> DatabaseManager::load(int year)
{
    std::shared_ptr<Data> data;

    while (loadQueue.try_dequeue(data)){
        logger->debug("Update cache for year {}.", data->year);
        cache[data->year] = data;
        // We don't put this in the request task because it runs on another thread,
        // otherwise we need a lock
        requesting.erase(data->year); 
    }

    if (cache.contains(year)) {
        logger->debug("Load cached data for year {}.", year);
        return cache[year];
    }

    logger->debug("No cached data found for year {}.", year);

    if (!requesting.contains(year)) {
        if (request(year)) {
            requesting.insert(year);
        }
    }

    return nullptr;
}

void DatabaseManager::remove(std::shared_ptr<Data> data)
{
    if (!taskQueue.enqueue([this, data](){
                                this->logger->debug("Process remove task for year {}.", data->year);
                                this->database.remove(*data);
                            })) {
        logger->error("Enqueue remove database year {} task fail.", data->year);
    }
}

void DatabaseManager::update(std::shared_ptr<Data> data)
{
    if (!taskQueue.enqueue([this, data](){
                            this->logger->debug("Process update task for year {}.", data->year);
                            this->database.upsert(*data);
                        })) {
        logger->error("Enqueue update database year {} task fail.", data->year);
    }
}

bool DatabaseManager::request(int year)
{
    if (!taskQueue.enqueue([this, year](){
                            this->logger->debug("Process load task for year {}.", year);
                            this->loadQueue.enqueue(std::make_shared<Data>(this->database.load(year)));
                        })) {
        logger->error("Enqueue request database year {} task fail.", year);

        return false;
    }

    return true;
}

size_t DatabaseManager::getWorkLoad()
{
    return taskQueue.size_approx();
}

DatabaseManager& DatabaseManager::getInstance()
{
    static DatabaseManager manager;
    return manager;
}

}