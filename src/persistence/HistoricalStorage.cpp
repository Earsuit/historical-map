#include "src/persistence/HistoricalStorage.h"

#include <cassert>

namespace persistence {
template<typename T>
auto mapKeyToVector(T&& constainer)
{
    std::vector<typename std::remove_cvref_t<T>::key_type> keys;
    for (const auto& [key, value] : constainer) {
        keys.emplace_back(key);
    }
    return keys;
}

void HistoricalStorage::constructCountryInfo()
{
    for (auto it = cache.countries.begin(); it != cache.countries.end(); it++) {
        countries.emplace(std::make_pair(it->name, it));
    }
}

void HistoricalStorage::constructCityInfo()
{
    for (auto it = cache.cities.begin(); it != cache.cities.end(); it++) {
        cities.emplace(std::make_pair(it->name, it));
    }
}

HistoricalStorage::HistoricalStorage(const persistence::Data& info):
    cache{info},
    removed{info.year}
{
    constructCountryInfo();
    constructCityInfo();
}

HistoricalStorage::HistoricalStorage(persistence::Data&& info):
    cache{std::move(info)},
    removed{info.year}
{
    constructCountryInfo();
    constructCityInfo();
}

persistence::Country& HistoricalStorage::getCountry(const std::string& name)
{
    return *countries[name];
}

persistence::City& HistoricalStorage::getCity(const std::string& name)
{
    return *cities[name];
}

persistence::Note& HistoricalStorage::getNote()
{
    return cache.note;
}

std::vector<std::string> HistoricalStorage::getCountryList() const
{
    return mapKeyToVector(countries);
}

std::vector<std::string> HistoricalStorage::getCityList() const
{
    return mapKeyToVector(cities);
}

void HistoricalStorage::removeCountry(const std::string& name)
{
    removed.countries.emplace_back(*countries[name]);
    cache.countries.erase(countries[name]);
    countries.erase(name);
}

void HistoricalStorage::removeCity(const std::string& name)
{
    removed.cities.emplace_back(*cities[name]);
    cache.cities.erase(cities[name]);
    cities.erase(name);
}

bool HistoricalStorage::addCountry(const std::string& name)
{
    if (countries.contains(name)) {
        return false;
    }

    cache.countries.emplace_back(persistence::Country{name});
    countries.emplace(std::make_pair(name, --cache.countries.end()));

    return true;
}

bool HistoricalStorage::addCity(const std::string& name, const persistence::Coordinate& coord)
{
    if (cities.contains(name)) {
        return false;
    }

    cache.cities.emplace_back(persistence::City{name, coord});
    cities.emplace(std::make_pair(name, --cache.cities.end()));

    return true;
}
}