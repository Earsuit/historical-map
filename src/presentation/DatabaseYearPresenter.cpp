#include "src/presentation/DatabaseYearPresenter.h"
#include "src/presentation/Util.h"
#include "src/logger/LoggerManager.h"

namespace presentation {
constexpr auto LOGGER_NAME = "DatabaseYearPresenter";

DatabaseYearPresenter::DatabaseYearPresenter():
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    cacheModel{model::CacheModel::getInstance()}
{
    util::signal::connect(&databaseModel,
                          &model::DatabaseModel::onYearChange,
                          this,
                          &DatabaseYearPresenter::updateInfo);
    
    updateInfo(databaseModel.getYear());
}

DatabaseYearPresenter::~DatabaseYearPresenter()
{
    util::signal::disconnectAll(&databaseModel,
                                &model::DatabaseModel::onYearChange,
                                this);
}

void DatabaseYearPresenter::handleMoveYearForward() noexcept
{
    databaseModel.moveYearForward();
}

void DatabaseYearPresenter::handleMoveYearBackward() noexcept
{
    databaseModel.moveYearBackward();
}

void DatabaseYearPresenter::handleSetYear(int year) noexcept
{
    databaseModel.setYear(year);
}

void DatabaseYearPresenter::updateInfo(int year)
{
    onYearChange(year);
    if (!worker.enqueue([this, year](){
            if (this->cacheModel.containsHistoricalInfo(model::PERMENANT_SOURCE, year)) {
                logger.debug("Historical info alredy exists in CacheModel from {} at year {}, skip it.", 
                              model::PERMENANT_SOURCE, 
                              year);
                return;
            }
            this->cacheModel.upsert(model::PERMENANT_SOURCE, 
                                    this->databaseModel.loadHistoricalInfo(year));
        })) {
        logger.error("Enqueue update historical info from database for year {} task fail.", year);
    }
}
}