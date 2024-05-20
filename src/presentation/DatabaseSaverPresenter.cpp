#include "src/presentation/DatabaseSaverPresenter.h"
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
    
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        progress = 0;
        total = endYear - startYear + 1;
        saveComplete = false;

        if (!taskQueue.enqueue([this, 
                                startYear, 
                                endYear,
                                data = info->getData(),
                                removed = info->getRemoved()] () mutable {
                for (int year = startYear; year <= endYear; year++) {
                    data.year = year;
                    removed.year = year;
                    this->databaseModel.updateHistoricalInfo(data);
                    this->databaseModel.removeHistoricalInfo(removed);
                    this->progress++;
                }

                saveComplete = true;
            })) {
            saveComplete = true;
            logger->error("Enqueue save same historical for range info modification to database task fail.");
        }

        return true;
    } else {
        logger->error("Current historical info load is null");
        return false;
    }
}

void DatabaseSaverPresenter::handleSaveAll()
{
    const auto years = this->dynamicInfoModel.getYearList(this->source);
    total = years.size();

    if (!taskQueue.enqueue([this, years] () mutable {
            for (const auto year : years) {
                if (auto info = this->dynamicInfoModel.getHistoricalInfo(this->source, year); info) {
                    this->databaseModel.updateHistoricalInfo(info->getData());
                    this->databaseModel.removeHistoricalInfo(info->getRemoved());
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