#ifndef SRC_MODEL_CACHE_MODEL_H
#define SRC_MODEL_CACHE_MODEL_H

#include "src/persistence/Data.h"
#include "src/persistence/HistoricalCache.h"
#include "src/logger/ModuleLogger.h"
#include "src/util/Signal.h"

#include <optional>
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <type_traits>
#include <set>

namespace model {
constexpr auto PERMENANT_SOURCE = "Database";

class CacheModel {
public:
    static CacheModel& getInstance();
    ~CacheModel();

    void setHoveredCoord(persistence::Coordinate coord);
    void clearHoveredCoord() noexcept;
    std::optional<persistence::Coordinate> getHoveredCoord() const;

    bool addSource(const std::string& source);
    std::vector<std::string> getSourceList() const noexcept;
    void removeSource(const std::string& source);
    void removeHistoricalInfoFromSource(const std::string& source, int year);
    std::vector<int> getYearList(const std::string& source) const;

    bool containsHistoricalInfo(const std::string& source, int year) const;
    bool containsCountry(const std::string& source, int year, const std::string& name) const;
    bool containsCity(const std::string& source, int year, const std::string& name) const;
    bool containsNote(const std::string& source, int year) const;
    std::optional<persistence::Country> getCountry(const std::string& source, int year, const std::string& name) const;
    std::optional<persistence::City> getCity(const std::string& source, int year, const std::string& name) const;
    std::optional<std::string> getNote(const std::string& source, int year) const;
    std::vector<std::string> getCountryList(const std::string& source, int year) const;
    std::vector<std::string> getCityList(const std::string& source, int year) const;
    std::list<persistence::Coordinate> getContour(const std::string& source, int year, const std::string& name) const;
    std::optional<persistence::Coordinate> getCityCoord(const std::string& source, int year, const std::string& name) const;
    bool extendContour(const std::string& source, int year, const std::string& name, const persistence::Coordinate& coord);
    bool delectFromContour(const std::string& source, int year, const std::string& name, int idx);
    bool updateContour(const std::string& source, int year, const std::string& name, int idx, const persistence::Coordinate& coord);
    bool updateCityCoord(const std::string& source, int year, const std::string& name, const persistence::Coordinate& coord);
    bool removeCountry(const std::string& source, int year, const std::string& name);
    bool removeCity(const std::string& source, int year, const std::string& name);
    bool removeNote(const std::string& source, int year);
    bool addCountry(const std::string& source, int year, const std::string& name);
    bool addCountry(const std::string& source, int year, const persistence::Country& country);
    bool addNote(const std::string& source, int year, const std::string& note);
    bool addCity(const std::string& source, int year, const persistence::City& city);
    std::optional<persistence::Data> getData(const std::string& source, int year) const noexcept;
    std::optional<persistence::Data> getRemoved(const std::string& source, int year) const noexcept;
    bool clearRemoved(const std::string& source, int year) noexcept;

    void setModificationState(const std::string& source, int year, bool isModified);
    bool isModified(const std::string& source, int year);

    template<typename T>
    requires (std::is_same_v<std::remove_cvref_t<T>, persistence::Data>)
    bool upsert(const std::string& source, T&& info)
    {
        std::lock_guard lk(cacheLock);
        if (cache.contains(source)) {
            logger.debug("Upsert CacheModel cache for source {} at year {}", source, info.year);

            for (const auto& city : info.cities) {
                if (cityToYear[source].contains(city.name)) {
                    cityToYear[source][city.name].emplace(info.year);
                } else {
                    cityToYear[source].emplace(std::make_pair(city.name, std::set<int>{info.year}));
                }
            }

            cache[source][info.year] = persistence::HistoricalCache{std::forward<T>(info)};

            onCountryUpdate(source, info.year);
            onCityUpdate(source, info.year);
            onNoteUpdate(source, info.year);
            onModificationChange(source, info.year, false);
            return true;
        }

        logger.error("Failed to upsert CacheModel cache for source {} at year {}, please add source first.", source, info.year);
        return false;
    }

    CacheModel(CacheModel&&) = delete;
    CacheModel(const CacheModel&) = delete;
    CacheModel& operator=(const CacheModel&) = delete;

    util::signal::Signal<void(const std::string& source, int year)> onCountryUpdate;
    util::signal::Signal<void(const std::string& source, int year)> onCityUpdate;
    util::signal::Signal<void(const std::string& source, int year)> onNoteUpdate;
    util::signal::Signal<void(const std::string& source, int year, bool)> onModificationChange;

private:
    CacheModel();

    mutable logger::ModuleLogger logger;
    std::optional<persistence::Coordinate> hovered;
    mutable std::mutex hoveredLock;
    mutable std::recursive_mutex cacheLock;
    std::map<std::string, std::map<int, persistence::HistoricalCache>> cache;
    // source -> city list -> years, track a city exists in which year
    std::map<std::string, std::map<std::string, std::set<int>>> cityToYear;
    persistence::Data removed;
};
}

#endif
