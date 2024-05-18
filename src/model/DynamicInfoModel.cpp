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
    logger->debug("Clear historical info from source {} at year {}", source, year);
    std::lock_guard lk(cacheLock);
    if (cache.contains(source)) {
        cache[source].erase(year);
    }
}

std::shared_ptr<persistence::HistoricalStorage> DynamicInfoModel::getHistoricalInfo(const std::string& source)
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source)) {
        return cache[source][year];
    }

    return nullptr;
}

bool DynamicInfoModel::containsHistoricalInfo(const std::string& source) const
{
    if (cache.contains(source) && cache.at(source).contains(year)) {
        return true;
    }

    return false;
}
}