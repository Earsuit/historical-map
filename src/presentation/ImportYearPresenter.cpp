#include "src/presentation/ImportYearPresenter.h"

#include "src/logger/Util.h"

#include <optional>

namespace presentation {
// the year list of source is not constucted because we haven't done import 
// when instantiate it
ImportYearPresenter::ImportYearPresenter(const std::string& source):
    logger{spdlog::get(logger::LOGGER_NAME)},
    dynamicInfoModel{model::DynamicInfoModel::getInstance()},
    source{source}
{
}

void ImportYearPresenter::initYearsList()
{
    if (years.empty()) {
        for (int year : dynamicInfoModel.getYearList(source)) {
            years.emplace(year);
        }
    }
}

int ImportYearPresenter::moveYearForward() noexcept
{
    initYearsList();
    if (auto it = years.upper_bound(dynamicInfoModel.getCurrentYear()); it != years.end()) {
        dynamicInfoModel.setCurrentYear(*it);
    }

    return dynamicInfoModel.getCurrentYear();
}

int ImportYearPresenter::moveYearBackward() noexcept
{
    initYearsList();
    if (auto it = years.lower_bound(dynamicInfoModel.getCurrentYear()); it != years.end() && it != years.begin()) {
        dynamicInfoModel.setCurrentYear(*std::prev(it));
    }

    return dynamicInfoModel.getCurrentYear();
}

int ImportYearPresenter::setYear(int year) noexcept
{
    auto lowerBound = years.lower_bound(year);
    
    if (lowerBound == years.begin()) {
        dynamicInfoModel.setCurrentYear(*lowerBound);
    } else if (lowerBound == years.end()) {
        dynamicInfoModel.setCurrentYear(*std::prev(lowerBound));
    } else {
        const auto prev = std::prev(lowerBound);
        const auto target = *lowerBound - year > year - *prev ? *prev : *lowerBound;
        dynamicInfoModel.setCurrentYear(target);
    }

    return dynamicInfoModel.getCurrentYear();
}

int ImportYearPresenter::getMaxYear() const noexcept
{
    return *years.rbegin();
}

int ImportYearPresenter::getMinYear() const noexcept
{
    return *years.begin();
}
}