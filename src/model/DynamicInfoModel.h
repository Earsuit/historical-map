#ifndef SRC_MODEL_DYNAMIC_INFO_MODEL_H
#define SRC_MODEL_DYNAMIC_INFO_MODEL_H

#include "src/persistence/Data.h"
#include "src/persistence/HistoricalStorage.h"
#include "src/logger/Util.h"
#include "src/util/Signal.h"

#include "spdlog/spdlog.h"

#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <atomic>
#include <type_traits>

namespace model {
class DynamicInfoModel {
public:
    static DynamicInfoModel& getInstance();

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

    template<typename T>
    requires (std::is_same_v<std::remove_cvref_t<T>, persistence::Data>)
    bool upsert(const std::string& source, T&& info)
    {
        std::lock_guard lk(cacheLock);
        if (cache.contains(source)) {
            logger->debug("Upsert DynamicInfoModel cache for source {} at year {}", source, info.year);
            cache[source][info.year] = persistence::HistoricalStorage{std::forward<T>(info)};

            onCountryUpdate(source, info.year);
            onCityUpdate(source, info.year);
            onNoteUpdate(source, info.year);
            return true;
        }

        logger->error("Failed to upsert DynamicInfoModel cache for source {} at year {}, please add source first.", source, info.year);
        return false;
    }

    DynamicInfoModel(DynamicInfoModel&&) = delete;
    DynamicInfoModel(const DynamicInfoModel&) = delete;
    DynamicInfoModel& operator=(const DynamicInfoModel&) = delete;

    util::signal::Signal<void(const std::string& source, int year)> onCountryUpdate;
    util::signal::Signal<void(const std::string& source, int year)> onCityUpdate;
    util::signal::Signal<void(const std::string& source, int year)> onNoteUpdate;

private:
    DynamicInfoModel():
        logger{spdlog::get(logger::LOGGER_NAME)}
    {
    }

    std::shared_ptr<spdlog::logger> logger;
    std::optional<persistence::Coordinate> hovered;
    mutable std::mutex hoveredLock;
    mutable std::recursive_mutex cacheLock;
    std::map<std::string, std::map<int, persistence::HistoricalStorage>> cache;
    persistence::Data removed;
};
}

#endif
