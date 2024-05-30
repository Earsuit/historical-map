#include "src/presentation/ImportYearPresenter.h"
#include "src/presentation/Util.h"

#include "src/logger/Util.h"

#include <optional>

namespace presentation {
// the year list of source is not constucted because we haven't done import 
// when instantiate it
ImportYearPresenter::ImportYearPresenter(const std::string& source):
    logger{spdlog::get(logger::LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    dynamicInfoModel{model::DynamicInfoModel::getInstance()},
    source{source}
{
    util::signal::connect(&databaseModel,
                          &model::DatabaseModel::onYearChange,
                          this,
                          &ImportYearPresenter::updateInfo);
}

ImportYearPresenter::~ImportYearPresenter()
{
    util::signal::disconnectAll(&databaseModel,
                                &model::DatabaseModel::onYearChange,
                                this);
}

void ImportYearPresenter::initYearsList()
{
    for (int year : dynamicInfoModel.getYearList(source)) {
        years.emplace(year);
    }
}

void ImportYearPresenter::handleMoveYearForward() noexcept
{
    if (auto it = years.upper_bound(databaseModel.getYear()); it != years.end()) {
        databaseModel.setYear(*it);
    }
}

void ImportYearPresenter::handleMoveYearBackward() noexcept
{
    if (auto it = years.lower_bound(databaseModel.getYear()); it != years.end() && it != years.begin()) {
        databaseModel.setYear(*std::prev(it));
    }
}

void ImportYearPresenter::handleSetYear(int year) noexcept
{
    auto lowerBound = years.lower_bound(year);
    
    if (lowerBound == years.begin()) {
        databaseModel.setYear(*lowerBound);
    } else if (lowerBound == years.end()) {
        databaseModel.setYear(*std::prev(lowerBound));
    } else {
        const auto prev = std::prev(lowerBound);
        const auto target = *lowerBound - year > year - *prev ? *prev : *lowerBound;
        databaseModel.setYear(target);
    }
}

int ImportYearPresenter::handleGetMaxYear() const noexcept
{
    return *years.rbegin();
}

int ImportYearPresenter::handleGetMinYear() const noexcept
{
    return *years.begin();
}

void ImportYearPresenter::updateInfo(int year)
{
    onYearChange(year);
    if (!worker.enqueue([this, year](){
            if (this->dynamicInfoModel.containsHistoricalInfo(DEFAULT_HISTORICAL_INFO_SOURCE, year)) {
                logger->debug("Historical info alredy exists in DynamicInfoModel from {} at year {}, skip it.", 
                              DEFAULT_HISTORICAL_INFO_SOURCE, 
                              year);
                return;
            }
            this->dynamicInfoModel.upsert(DEFAULT_HISTORICAL_INFO_SOURCE, 
                                          this->databaseModel.loadHistoricalInfo(year));
        })) {
        logger->error("Enqueue update historical info from database for year {} task fail.", year);
    }
}
}