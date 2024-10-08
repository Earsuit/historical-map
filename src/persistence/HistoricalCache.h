#ifndef SRC_PERSISTENCE_HISTORICAL_CACHE_H
#define SRC_PERSISTENCE_HISTORICAL_CACHE_H

#include "src/persistence/Data.h"

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <vector>

namespace persistence {
class HistoricalCache {
public:
    HistoricalCache() = default;
    explicit HistoricalCache(const persistence::Data& info);
    explicit HistoricalCache(persistence::Data&& info);

    bool containsCountry(const std::string& name) const { return countries.contains(name); }
    bool containsCity(const std::string& name) const { return cities.contains(name); }
    bool containsNote() const noexcept { return cache.note.has_value(); }
    const persistence::Country& getCountry(const std::string& name) const;
    const persistence::City& getCity(const std::string& name) const;
    const persistence::Note& getNote() const;
    persistence::Country& getCountry(const std::string& name);
    persistence::City& getCity(const std::string& name);
    persistence::Note& getNote();
    std::vector<std::string> getCountryList() const;
    std::vector<std::string> getCityList() const;
    void removeCountry(const std::string& name);
    void removeCity(const std::string& name);
    void removeNote();
    bool addCountry(const std::string& name);
    bool addCountry(const persistence::Country& country);
    bool addNote(const std::string& note);
    bool addCity(const persistence::City& city);
    persistence::Data getData() const noexcept { return cache; }
    persistence::Data getRemoved() const noexcept { return removed; }
    void clearRemoved() noexcept { removed = persistence::Data{cache.year}; }

    void setModificationState(bool isModified) noexcept { modified = isModified; }
    bool isMidified() const noexcept { return modified; }

private:
    persistence::Data cache;
    persistence::Data removed;
    std::map<std::string, std::list<persistence::Country>::iterator> countries;
    std::map<std::string, std::list<persistence::City>::iterator> cities;
    // we don't track if it is the same as the database
    // once it is modified, even though it may be modified back to the same 
    // as the counterpart in the database, we still treat it as modified.
    bool modified = false;

    void constructCountryInfo();
    void constructCityInfo();
};
}

#endif
