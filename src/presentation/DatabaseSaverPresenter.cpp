#include "src/presentation/DatabaseSaverPresenter.h"
#include "src/presentation/Util.h"
#include "src/logger/Util.h"

namespace presentation {
DatabaseSaverPresenter::DatabaseSaverPresenter(const std::string& source):
    logger{spdlog::get(logger::LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    dynamicInfoModel{model::DynamicInfoModel::getInstance()},
    source{source}
{
    startWorkerThread();
}

bool DatabaseSaverPresenter::handleSaveSameForRange(int startYear, int endYear)
{
    if (startYear > endYear) {
        logger->error("Start year must less than end year");
        return false;
    }
    
    progress = 0;
    total = endYear - startYear + 1;
    saveComplete = false;

    if (!taskQueue.enqueue([this, 
                            startYear, 
                            endYear] () mutable {
            const auto year = this->databaseModel.getYear();
            auto data = this->dynamicInfoModel.getData(this->source, year);
            auto removed = this->dynamicInfoModel.getRemoved(this->source, year);
            if (data && removed) {
                for (int year = startYear; year <= endYear; year++) {
                    data->year = year;
                    removed->year = year;
                    this->databaseModel.updateHistoricalInfo(*data);
                    this->databaseModel.removeHistoricalInfo(*removed);
                    this->dynamicInfoModel.upsert(DEFAULT_HISTORICAL_INFO_SOURCE, this->databaseModel.loadHistoricalInfo(year));
                    this->progress++;
                }
            }

            saveComplete = true;
        })) {
        saveComplete = true;
        logger->error("Enqueue save same historical for range info modification to database task fail.");
    }

    return true;
}

void DatabaseSaverPresenter::handleSaveAll()
{
    const auto years = this->dynamicInfoModel.getYearList(this->source);
    total = years.size();

    if (!taskQueue.enqueue([this, years] () mutable {
            for (const auto year : years) {
                auto data = this->dynamicInfoModel.getData(this->source, year);
                auto removed = this->dynamicInfoModel.getRemoved(this->source, year);

                if (data && removed) {
                    this->databaseModel.updateHistoricalInfo(*data);
                    this->databaseModel.removeHistoricalInfo(*removed);
                    this->dynamicInfoModel.upsert(DEFAULT_HISTORICAL_INFO_SOURCE, this->databaseModel.loadHistoricalInfo(year));
                }
                this->progress++;
            }

            saveComplete = true;
        })) {
        saveComplete = true;
        logger->error("Enqueue save all historical modificcation info modification to database task fail.");
    }
}

float DatabaseSaverPresenter::getProgress() const noexcept
{
    return progress / static_cast<float>(total);
}

void DatabaseSaverPresenter::startWorkerThread()
{
    runWorkerThread = true;
    workerThread = std::thread(&DatabaseSaverPresenter::worker, this);
}

void DatabaseSaverPresenter::stopWorkerThread()
{
    runWorkerThread = false;

    // enqueue an empty task to wake up the wait_dequeue if necessary
    taskQueue.enqueue([](){});

    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void DatabaseSaverPresenter::worker()
{
    while (runWorkerThread) {
        std::function<void()> task; 

        taskQueue.wait_dequeue(task);

        task();
    }
}
}