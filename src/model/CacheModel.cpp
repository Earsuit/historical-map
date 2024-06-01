#include "src/model/CacheModel.h"

#include <ranges>

namespace model {
CacheModel& CacheModel::getInstance()
{
    static CacheModel model;
    return model;
}

void CacheModel::setHoveredCoord(persistence::Coordinate coord)
{
    std::lock_guard lk(hoveredLock);
    hovered = coord;
}

void CacheModel::clearHoveredCoord() noexcept
{
    std::lock_guard lk(hoveredLock);
    hovered.reset();
}

std::optional<persistence::Coordinate> CacheModel::getHoveredCoord() const
{
    std::lock_guard lk(hoveredLock);
    return hovered;
}

bool CacheModel::addSource(const std::string& source)
{
    logger->debug("Add source {}", source);
    std::lock_guard lk(cacheLock);
    if (!cache.contains(source)) {
        cache.emplace(std::make_pair(source, std::map<int, persistence::HistoricalCache>{}));
        return true;
    }

    return false;
}

std::vector<std::string> CacheModel::getSourceList() const noexcept
{
    std::lock_guard lk(cacheLock);
    std::vector<std::string> keys;
    for (const auto& key : std::views::keys(cache)) {
        keys.emplace_back(key);
    }

    return keys;
}

void CacheModel::removeSource(const std::string& source)
{
    logger->debug("Remove source {}", source);
    std::lock_guard lk(cacheLock);
    cache.erase(source);
}

void CacheModel::removeHistoricalInfoFromSource(const std::string& source, int year)
{
    logger->debug("Clear historical info from source {} at year {}", source, year);
    std::lock_guard lk(cacheLock);
    if (cache.contains(source)) {
        cache[source].erase(year);
        onCountryUpdate(source, year);
        onCityUpdate(source, year);
        onNoteUpdate(source, year);
    }
}

std::vector<int> CacheModel::getYearList(const std::string& source) const
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

bool CacheModel::containsHistoricalInfo(const std::string& source, int year) const
{
    std::lock_guard lk(cacheLock);
    if (cache.contains(source) && cache.at(source).contains(year)) {
        return true;
    }

    return false;
}

bool CacheModel::containsCountry(const std::string& source, int year, const std::string& name) const
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source, year)) {
        return cache.at(source).at(year).containsCountry(name);
    }

    return false;
}

bool CacheModel::containsCity(const std::string& source, int year, const std::string& name) const
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source, year)) {
        return cache.at(source).at(year).containsCity(name);
    }

    return false;
}

bool CacheModel::containsNote(const std::string& source, int year) const
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source, year)) {
        return cache.at(source).at(year).containsNote();
    }

    return false;
}

std::optional<persistence::Country> CacheModel::getCountry(const std::string& source, int year, const std::string& name) const
{
    std::lock_guard lk(cacheLock);
    if (containsCountry(source, year, name)) {
        return cache.at(source).at(year).getCountry(name);
    }

    return std::nullopt;
}

std::optional<persistence::City> CacheModel::getCity(const std::string& source, int year, const std::string& name) const
{
    std::lock_guard lk(cacheLock);
    if (containsCity(source, year, name)) {
        return cache.at(source).at(year).getCity(name);
    }

    return std::nullopt;
}

std::vector<std::string> CacheModel::getCountryList(const std::string& source, int year) const
{
    std::lock_guard lk(cacheLock);

    if (containsHistoricalInfo(source, year)) {
        return cache.at(source).at(year).getCountryList();
    }

    return {};
}

std::vector<std::string> CacheModel::getCityList(const std::string& source, int year) const
{
    std::lock_guard lk(cacheLock);

    if (containsHistoricalInfo(source, year)) {
        return cache.at(source).at(year).getCityList();
    }

    return {};
}

std::list<persistence::Coordinate> CacheModel::getContour(const std::string& source, int year, const std::string& name) const
{
    std::lock_guard lk(cacheLock);

    if (containsCountry(source, year, name)) {
        return cache.at(source).at(year).getCountry(name).borderContour;
    }

    return {};
}

std::optional<persistence::Coordinate> CacheModel::getCityCoord(const std::string& source, int year, const std::string& name) const
{
    std::lock_guard lk(cacheLock);

    if (containsCity(source, year, name)) {
        return cache.at(source).at(year).getCity(name).coordinate;
    }

    return std::nullopt;
}

std::optional<std::string> CacheModel::getNote(const std::string& source, int year) const
{
    std::lock_guard lk(cacheLock);
    if (containsNote(source, year)) {
        return cache.at(source).at(year).getNote().text;
    }

    return std::nullopt;
}

bool CacheModel::extendContour(const std::string& source, int year, const std::string& name, const persistence::Coordinate& coord)
{
    std::lock_guard lk(cacheLock);

    if (containsCountry(source, year, name)) {
        auto& country = cache.at(source).at(year).getCountry(name);
        country.borderContour.emplace_back(coord);
        onCountryUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::delectFromContour(const std::string& source, int year, const std::string& name, int idx)
{
    std::lock_guard lk(cacheLock);

    if (containsCountry(source, year, name)) {
        auto& contour = cache.at(source).at(year).getCountry(name).borderContour;
        if (idx < contour.size()) {
            auto it = std::next(contour.begin(), idx);
            contour.erase(it);
            onCountryUpdate(source, year);
            return true;
        }
    }

    return false;
}

bool CacheModel::updateContour(const std::string& source, int year, const std::string& name, int idx, const persistence::Coordinate& coord)
{
    std::lock_guard lk(cacheLock);

    if (containsCountry(source, year, name)) {
        auto& contour = cache.at(source).at(year).getCountry(name).borderContour;
        auto it = std::next(contour.begin(), idx);
        *it = coord;
        onCountryUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::updateCityCoord(const std::string& source, int year, const std::string& name, const persistence::Coordinate& coord)
{
    std::lock_guard lk(cacheLock);

    if (containsCity(source, year, name)) {
        auto& city = cache.at(source).at(year).getCity(name);
        city.coordinate = coord;
        onCityUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::removeCountry(const std::string& source, int year, const std::string& name)
{
    std::lock_guard lk(cacheLock);

    if (containsCountry(source, year, name)) {
        cache.at(source).at(year).removeCountry(name);
        onCountryUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::removeCity(const std::string& source, int year, const std::string& name)
{
    std::lock_guard lk(cacheLock);

    if (containsCity(source, year, name)) {
        cache.at(source).at(year).removeCity(name);
        onCityUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::removeNote(const std::string& source, int year)
{
    std::lock_guard lk(cacheLock);

    if (containsNote(source, year)) {
        cache.at(source).at(year).removeNote();
        onNoteUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::addCountry(const std::string& source, int year, const std::string& name)
{
    std::lock_guard lk(cacheLock);

    if (containsHistoricalInfo(source, year)) {
        if (cache.at(source).at(year).addCountry(name)) {
            onCountryUpdate(source, year);
            return true;
        }
    }

    return false;
}

bool CacheModel::addCountry(const std::string& source, int year, const persistence::Country& country)
{
    std::lock_guard lk(cacheLock);

    if (containsHistoricalInfo(source, year)) {
        if (cache.at(source).at(year).addCountry(country)) {
            onCountryUpdate(source, year);
            return true;
        }
    }

    return false;
}

bool CacheModel::addNote(const std::string& source, int year, const std::string& note)
{
    std::lock_guard lk(cacheLock);

    if (containsHistoricalInfo(source, year)) {
        if (cache.at(source).at(year).addNote(note)) {
            onNoteUpdate(source, year);
            return true;
        }
    }

    return false;
}

bool CacheModel::addCity(const std::string& source, int year, const persistence::City& city)
{
    std::lock_guard lk(cacheLock);

    if (containsHistoricalInfo(source, year)) {
        if (cache.at(source).at(year).addCity(city)) {
            onCityUpdate(source, year);
            return true;
        } 
    }

    return false;
}

std::optional<persistence::Data> CacheModel::getData(const std::string& source, int year) const noexcept
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source, year)) {
        return cache.at(source).at(year).getData();
    }

    return std::nullopt;
}

std::optional<persistence::Data> CacheModel::getRemoved(const std::string& source, int year) const noexcept
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source, year)) {
        return cache.at(source).at(year).getRemoved();
    }

    return std::nullopt;
}

bool CacheModel::clearRemoved(const std::string& source, int year) noexcept
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source, year)) {
        cache.at(source).at(year).clearRemoved();
        return true;
    }

    return false;
}
}