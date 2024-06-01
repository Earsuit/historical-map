#include "src/presentation/DatabaseYearPresenter.h"
#include "src/presentation/Util.h"
#include "src/logger/Util.h"

namespace presentation {
DatabaseYearPresenter::DatabaseYearPresenter():
    logger{spdlog::get(logger::LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    cacheModel{model::CacheModel::getInstance()}
{
    util::signal::connect(&databaseModel,
                          &model::DatabaseModel::onYearChange,
                          this,
                          &DatabaseYearPresenter::updateInfo);

    if (!cacheModel.addSource(DEFAULT_HISTORICAL_INFO_SOURCE)) {
        logger->debug("Source {} already exists in CacheModel", DEFAULT_HISTORICAL_INFO_SOURCE);
    }
    
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
            if (this->cacheModel.containsHistoricalInfo(DEFAULT_HISTORICAL_INFO_SOURCE, year)) {
                logger->debug("Historical info alredy exists in CacheModel from {} at year {}, skip it.", 
                              DEFAULT_HISTORICAL_INFO_SOURCE, 
                              year);
                return;
            }
            this->cacheModel.upsert(DEFAULT_HISTORICAL_INFO_SOURCE, 
                                          this->databaseModel.loadHistoricalInfo(year));
        })) {
        logger->error("Enqueue update historical info from database for year {} task fail.", year);
    }
}
}