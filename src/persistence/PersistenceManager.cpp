#include "src/persistence/PersistenceManager.h"
#include "src/logger/Util.h"

namespace persistence {
constexpr auto DATABASE_NAME = "HistoricalMapDB";
constexpr auto QUEUE_SIZE = 128;
constexpr auto DATA_CACHE_SIZE = 8;

PersistenceManager::PersistenceManager():
    persistence{std::make_shared<sqlpp::sqlite3::connection_config>(DATABASE_NAME, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)},
    taskQueue{QUEUE_SIZE},
    loadQueue{QUEUE_SIZE},
    logger{spdlog::get(logger::LOGGER_NAME)},
    cache{DATA_CACHE_SIZE}
{
    startWorkerThread();
}

PersistenceManager::~PersistenceManager()
{
    stopWorkerThread();
}

void PersistenceManager::startWorkerThread()
{
    runWorkerThread = true;
    workerThread = std::thread(&PersistenceManager::worker, this);
}

void PersistenceManager::stopWorkerThread()
{
    runWorkerThread = false;

    // enqueue an empty task to wake up the wait_dequeue if necessary
    taskQueue.enqueue([](){});

    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void PersistenceManager::worker()
{
    while (runWorkerThread) {
        std::function<void()> task; 

        taskQueue.wait_dequeue(task);

        task();
    }
}

std::optional<Data> PersistenceManager::load(int year)
{
    Data data;

    while (loadQueue.try_dequeue(data)){
        logger->debug("Update cache for year {}.", data.year);
        cache[data.year] = data;
    }

    if (cache.contains(year)) {
        logger->debug("Load cached data for year {}.", year);
        return cache[year];
    }

    logger->debug("No cached data found for year {}.", year);

    request(year);
    
    return std::nullopt;
}

void PersistenceManager::remove(const Data data)
{
    if (!taskQueue.enqueue([this, data = std::move(data)](){
                                this->logger->debug("Process remove task for year {}.", data.year);
                                this->persistence.remove(data);
                            })) {
        logger->error("Enqueue remove database year {} task fail.", data.year);
    }
}

void PersistenceManager::update(const Data data)
{
    // overwrite the cached data
    cache[data.year] = data;

    if (!taskQueue.enqueue([this, data = std::move(data)](){
                            this->logger->debug("Process update task for year {}.", data.year);
                            this->persistence.upsert(data);
                        })) {
        logger->error("Enqueue update database year {} task fail.", data.year);
    }
}

void PersistenceManager::request(int year)
{
    if (!taskQueue.enqueue([this, year](){
                            this->logger->debug("Process load task for year {}.", year);
                            this->loadQueue.emplace(this->persistence.load(year));
                        })) {
        logger->error("Enqueue request database year {} task fail.", year);
    }
}

}