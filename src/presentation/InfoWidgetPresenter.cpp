#include "src/presentation/InfoWidgetPresenter.h"
#include "src/logger/Util.h"

namespace presentation {
constexpr auto DEFAULT_SOURCE = "Database";

InfoWidgetPresenter::InfoWidgetPresenter():
    logger{spdlog::get(logger::LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    dynamicInfoModel{model::DynamicInfoModel::getInstance()}
{
    if (!dynamicInfoModel.addSource(DEFAULT_SOURCE)) {
        logger->critical("Failed to add source {}", DEFAULT_SOURCE);
    }
    startWorkerThread();
    updateInfo();
}

void InfoWidgetPresenter::handleMoveYearForward() noexcept
{
    databaseModel.moveYearForward();
    updateInfo();
}

void InfoWidgetPresenter::handleMoveYearBackward() noexcept
{
    databaseModel.moveYearBackward();
    updateInfo();
}

void InfoWidgetPresenter::handleSetYear(int year) noexcept
{
    databaseModel.setYear(year);
    updateInfo();
}

int InfoWidgetPresenter::getYear() const noexcept
{
    return databaseModel.getYear();
}

void InfoWidgetPresenter::startWorkerThread()
{
    runWorkerThread = true;
    workerThread = std::thread(&InfoWidgetPresenter::worker, this);
}

void InfoWidgetPresenter::stopWorkerThread()
{
    runWorkerThread = false;

    // enqueue an empty task to wake up the wait_dequeue if necessary
    taskQueue.enqueue([](){});

    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void InfoWidgetPresenter::worker()
{
    while (runWorkerThread) {
        std::function<void()> task; 

        taskQueue.wait_dequeue(task);

        task();
    }
}

void InfoWidgetPresenter::updateInfo()
{
    if (!taskQueue.enqueue([this](){
                                this->dynamicInfoModel.upsert(DEFAULT_SOURCE, 
                                                              this->databaseModel.loadHistoricalInfo());
                            })) {
        logger->error("Enqueue update historical info from database for year {} task fail.", databaseModel.getYear());
    }
}
}