#include "src/persistence/HistoricalCache.h"

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

void HistoricalCache::constructCountryInfo()
{
    for (auto it = cache.countries.begin(); it != cache.countries.end(); it++) {
        countries.emplace(std::make_pair(it->name, it));
    }
}

void HistoricalCache::constructCityInfo()
{
    for (auto it = cache.cities.begin(); it != cache.cities.end(); it++) {
        cities.emplace(std::make_pair(it->name, it));
    }
}

HistoricalCache::HistoricalCache(const persistence::Data& info):
    cache{info},
    removed{info.year}
{
    constructCountryInfo();
    constructCityInfo();
}

HistoricalCache::HistoricalCache(persistence::Data&& info):
    cache{std::move(info)},
    removed{info.year}
{
    constructCountryInfo();
    constructCityInfo();
}

persistence::Country& HistoricalCache::getCountry(const std::string& name)
{
    return *countries.at(name);
}

persistence::City& HistoricalCache::getCity(const std::string& name)
{
    return *cities.at(name);
}

persistence::Note& HistoricalCache::getNote()
{
    return *cache.note;
}

const persistence::Country& HistoricalCache::getCountry(const std::string& name) const
{
    return *countries.at(name);
}

const persistence::City& HistoricalCache::getCity(const std::string& name) const
{
    return *cities.at(name);
}

const persistence::Note& HistoricalCache::getNote() const
{
    return *cache.note;
}

std::vector<std::string> HistoricalCache::getCountryList() const
{
    return mapKeyToVector(countries);
}

std::vector<std::string> HistoricalCache::getCityList() const
{
    return mapKeyToVector(cities);
}

void HistoricalCache::removeCountry(const std::string& name)
{
    removed.countries.emplace_back(*countries[name]);
    cache.countries.erase(countries[name]);
    countries.erase(name);
}

void HistoricalCache::removeCity(const std::string& name)
{
    removed.cities.emplace_back(*cities[name]);
    cache.cities.erase(cities[name]);
    cities.erase(name);
}

bool HistoricalCache::addCountry(const std::string& name)
{
    return addCountry(persistence::Country{name});
}

bool HistoricalCache::addCountry(const persistence::Country& country)
{
    if (countries.contains(country.name)) {
        return false;
    }

    cache.countries.emplace_back(country);
    countries.emplace(std::make_pair(country.name, --cache.countries.end()));

    return true;
}

bool HistoricalCache::addCity(const persistence::City& city)
{
    if (cities.contains(city.name)) {
        return false;
    }

    cache.cities.emplace_back(city);
    cities.emplace(std::make_pair(city.name, --cache.cities.end()));

    return true;
}

bool HistoricalCache::addNote(const std::string& note)
{
    if (cache.note) {
        cache.note->text = note;
    } else {
        cache.note = persistence::Note{note};
    }

    return true;
}

void HistoricalCache::removeNote()
{
    removed.note = cache.note;
    cache.note = std::nullopt;
}
}