#ifndef SRC_PERSISTENCE_HISTORICAL_STORAGE_H
#define SRC_PERSISTENCE_HISTORICAL_STORAGE_H

#include "src/persistence/Data.h"

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <vector>

namespace persistence {
class HistoricalStorage {
public:
    HistoricalStorage() = default;
    explicit HistoricalStorage(const persistence::Data& info);
    explicit HistoricalStorage(persistence::Data&& info);

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

private:
    persistence::Data cache;
    persistence::Data removed;
    std::map<std::string, std::list<persistence::Country>::iterator> countries;
    std::map<std::string, std::list<persistence::City>::iterator> cities;

    void constructCountryInfo();
    void constructCityInfo();
};
}

#endif
