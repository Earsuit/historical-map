#include "src/model/CacheModel.h"

#include <ranges>

namespace model {
CacheModel& CacheModel::getInstance()
{
    static CacheModel model;
    return model;
}

CacheModel::CacheModel():
    logger{spdlog::get(logger::LOGGER_NAME)}
{
    addSource(PERMENANT_SOURCE);
    util::signal::connect(this,
                          &CacheModel::onModificationChange,
                          this,
                          &CacheModel::setModificationState);
}

CacheModel::~CacheModel()
{
    util::signal::disconnectAll(this,
                                &CacheModel::onModificationChange,
                                this);
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
        cityToYear.emplace(std::make_pair(source, std::map<std::string, std::set<int>>{}));
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
    if (source == PERMENANT_SOURCE) {
        logger->debug("Trying to remove permanent source {}, abort.", PERMENANT_SOURCE);
        return;
    }

    logger->debug("Remove source {}", source);
    std::lock_guard lk(cacheLock);
    cache.erase(source);
    cityToYear.erase(source);
}

void CacheModel::removeHistoricalInfoFromSource(const std::string& source, int year)
{
    logger->debug("Clear historical info from source {} at year {}", source, year);
    std::lock_guard lk(cacheLock);
    if (cache.contains(source)) {
        if (containsHistoricalInfo(source, year)) {
            for (const auto& city : cache[source][year].getCityList()) {
                cityToYear[source][city].erase(year);

                if (cityToYear[source][city].empty()) {
                    cityToYear[source].erase(city);
                }
            }
        }

        cache[source].erase(year);

        onModificationChange(source, year, false);
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
        auto& infoCache = cache.at(source).at(year);
        auto& country = infoCache.getCountry(name);
        country.borderContour.emplace_back(coord);
        onModificationChange(source, year, true);
        onCountryUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::delectFromContour(const std::string& source, int year, const std::string& name, int idx)
{
    std::lock_guard lk(cacheLock);

    if (containsCountry(source, year, name)) {
        auto& infoCache = cache.at(source).at(year);
        auto& contour = infoCache.getCountry(name).borderContour;
        if (idx < contour.size()) {
            auto it = std::next(contour.begin(), idx);
            contour.erase(it);
            onModificationChange(source, year, true);
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
        auto& infoCache = cache.at(source).at(year);
        auto& contour = infoCache.getCountry(name).borderContour;
        auto it = std::next(contour.begin(), idx);
        *it = coord;
        onModificationChange(source, year, true);
        onCountryUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::updateCityCoord(const std::string& source, int year, const std::string& name, const persistence::Coordinate& coord)
{
    std::lock_guard lk(cacheLock);

    if (containsCity(source, year, name)) {
        for (const auto cityAtYear : cityToYear[source][name]) {
            if (!containsCity(source, cityAtYear, name)) {
                // it should exists
                logger->error("Cache of source {} at year {} doesn't exist when updateing the city {} coord",
                              source, 
                              cityAtYear, 
                              name);
                continue;
            }

            auto& infoCache = cache.at(source).at(cityAtYear);
            auto& city = infoCache.getCity(name);
            city.coordinate = coord;
            onModificationChange(source, cityAtYear, true);
            onCityUpdate(source, cityAtYear);
        }
        
        return true;
    }

    return false;
}

bool CacheModel::removeCountry(const std::string& source, int year, const std::string& name)
{
    std::lock_guard lk(cacheLock);

    if (containsCountry(source, year, name)) {
        auto& infoCache = cache.at(source).at(year);
        infoCache.removeCountry(name);
        onModificationChange(source, year, true);
        onCountryUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::removeCity(const std::string& source, int year, const std::string& name)
{
    std::lock_guard lk(cacheLock);

    if (containsCity(source, year, name)) {
        if (cityToYear[source].contains(name)) {
            cityToYear[source][name].erase(year);

            if (cityToYear[source][name].empty()) {
                cityToYear[source].erase(name);
            }
        }

        auto& infoCache = cache.at(source).at(year);
        infoCache.removeCity(name);
        onModificationChange(source, year, true);
        onCityUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::removeNote(const std::string& source, int year)
{
    std::lock_guard lk(cacheLock);

    if (containsNote(source, year)) {
        auto& infoCache = cache.at(source).at(year);
        infoCache.removeNote();
        onModificationChange(source, year, true);
        onNoteUpdate(source, year);
        return true;
    }

    return false;
}

bool CacheModel::addCountry(const std::string& source, int year, const std::string& name)
{
    std::lock_guard lk(cacheLock);

    if (containsHistoricalInfo(source, year)) {
        auto& infoCache = cache.at(source).at(year);
        if (infoCache.addCountry(name)) {
            onModificationChange(source, year, true);
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
        auto& infoCache = cache.at(source).at(year);
        if (infoCache.addCountry(country)) {
            onModificationChange(source, year, true);
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
        auto& infoCache = cache.at(source).at(year);
        if (infoCache.addNote(note)) {
            onModificationChange(source, year, true);
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
        auto& infoCache = cache.at(source).at(year);
        if (infoCache.addCity(city)) {
            if (cityToYear[source].contains(city.name)) {
                cityToYear[source][city.name].emplace(year);
            } else {
                cityToYear[source].emplace(std::make_pair(city.name, std::set<int>{year}));
            }

            onModificationChange(source, year, true);
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

void CacheModel::setModificationState(const std::string& source, int year, bool isModified)
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source, year)) {
        cache.at(source).at(year).setModificationState(isModified);
    }
}

bool CacheModel::isModified(const std::string& source, int year)
{
    std::lock_guard lk(cacheLock);
    if (containsHistoricalInfo(source, year)) {
        return cache.at(source).at(year).isMidified();
    }

    return false;
}
}