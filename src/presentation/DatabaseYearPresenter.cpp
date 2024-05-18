#include "src/presentation/DatabaseYearPresenter.h"
#include "src/presentation/Util.h"
#include "src/logger/Util.h"

namespace presentation {
DatabaseYearPresenter::DatabaseYearPresenter():
    logger{spdlog::get(logger::LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    dynamicInfoModel{model::DynamicInfoModel::getInstance()}
{
    if (!dynamicInfoModel.addSource(DEFAULT_HISTORICAL_INFO_SOURCE)) {
        logger->critical("Failed to add source {}", DEFAULT_HISTORICAL_INFO_SOURCE);
    }
    startWorkerThread();
    updateInfo();
}

void DatabaseYearPresenter::handleMoveYearForward() noexcept
{
    databaseModel.moveYearForward();
    updateInfo();
}

void DatabaseYearPresenter::handleMoveYearBackward() noexcept
{
    databaseModel.moveYearBackward();
    updateInfo();
}

void DatabaseYearPresenter::handleSetYear(int year) noexcept
{
    databaseModel.setYear(year);
    updateInfo();
}

int DatabaseYearPresenter::getYear() const noexcept
{
    return dynamicInfoModel.getCurrentYear();
}

void DatabaseYearPresenter::startWorkerThread()
{
    runWorkerThread = true;
    workerThread = std::thread(&DatabaseYearPresenter::worker, this);
}

void DatabaseYearPresenter::stopWorkerThread()
{
    runWorkerThread = false;

    // enqueue an empty task to wake up the wait_dequeue if necessary
    taskQueue.enqueue([](){});

    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void DatabaseYearPresenter::worker()
{
    while (runWorkerThread) {
        std::function<void()> task; 

        taskQueue.wait_dequeue(task);

        task();
    }
}

void DatabaseYearPresenter::updateInfo()
{
    dynamicInfoModel.setCurrentYear(databaseModel.getYear());
    if (!taskQueue.enqueue([this](){
            if (this->dynamicInfoModel.containsHistoricalInfo(DEFAULT_HISTORICAL_INFO_SOURCE)) {
                logger->debug("Historical info alredy exists in DynamicInfoModel from {} at year {}, skip it.", 
                              DEFAULT_HISTORICAL_INFO_SOURCE, 
                              this->databaseModel.getYear());
                return;
            }
            this->dynamicInfoModel.upsert(DEFAULT_HISTORICAL_INFO_SOURCE, 
                                          this->databaseModel.loadHistoricalInfo());
        })) {
        logger->error("Enqueue update historical info from database for year {} task fail.", databaseModel.getYear());
    }
}
}