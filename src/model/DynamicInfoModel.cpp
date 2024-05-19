#include "src/model/DynamicInfoModel.h"

#include <ranges>

namespace model {
DynamicInfoModel& DynamicInfoModel::getInstance()
{
    static DynamicInfoModel model;
    return model;
}

void DynamicInfoModel::setHoveredCoord(persistence::Coordinate coord)
{
    std::lock_guard lk(hoveredLock);
    hovered = coord;
}

void DynamicInfoModel::clearHoveredCoord() noexcept
{
    std::lock_guard lk(hoveredLock);
    hovered.reset();
}

std::optional<persistence::Coordinate> DynamicInfoModel::getHoveredCoord() const
{
    std::lock_guard lk(hoveredLock);
    return hovered;
}

bool DynamicInfoModel::addSource(const std::string& source)
{
    logger->debug("Add source {}", source);
    std::lock_guard lk(cacheLock);
    if (!cache.contains(source)) {
        cache.emplace(std::make_pair(source, std::map<int, std::shared_ptr<persistence::HistoricalStorage>>{}));
        return true;
    }

    return false;
}

std::vector<std::string> DynamicInfoModel::getSourceList() const noexcept
{
    std::lock_guard lk(cacheLock);
    std::vector<std::string> keys;
    for (const auto& key : std::views::keys(cache)) {
        keys.emplace_back(key);
    }

    return keys;
}

void DynamicInfoModel::removeSource(const std::string& source)
{
    logger->debug("Remove source {}", source);
    std::lock_guard lk(cacheLock);
    cache.erase(source);
}

void DynamicInfoModel::removeHistoricalInfoFromSource(const std::string& source)
{
    logger->debug("Clear historical info from source {} at year {}", source, currentYear);
    std::lock_guard lk(cacheLock);
    if (cache.contains(source)) {
        cache[source].erase(currentYear);
    }
}

std::shared_ptr<persistence::HistoricalStorage> DynamicInfoModel::getHistoricalInfo(const std::string& source)
{
    return getHistoricalInfo(source, currentYear);
}

std::shared_ptr<persistence::HistoricalStorage> DynamicInfoModel::getHistoricalInfo(const std::string& source, int year)
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source, year)) {
        return cache[source][year];
    }

    return nullptr;
}

std::vector<int> DynamicInfoModel::getYearList(const std::string& source) const
{
    std::lock_guard lk(cacheLock);
    std::vector<int> years;
    if (cache.contains(source)) {
        years.reserve(cache.at(source).size());
        for (const auto& [year, info] : cache.at(source)) {
            years.emplace_back(year);
        }
    } else {
        logger->error("Failed to get year list for source {} because it doesn't exists", source);
    }

    return years;
}

bool DynamicInfoModel::containsHistoricalInfo(const std::string& source, int year) const
{
    if (cache.contains(source) && cache.at(source).contains(year)) {
        return true;
    }

    return false;
}
}