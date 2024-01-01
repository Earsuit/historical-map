#ifndef SRC_PERSISTENCE_PERSISTENCE_H
#define SRC_PERSISTENCE_PERSISTENCE_H

#include "src/persistence/Data.h"

// These are generated headers
#include "src/persistence/Commands.h"
#include "src/persistence/Table.h"

#include "sqlpp11/sqlpp11.h"

#include <memory>
#include <future>
#include <utility>
#include <future>
#include<tr2/type_traits>

namespace persistence {

// A template to extract and hold the arguments
template <typename T>
struct extract_table_members;

// Specializing the template for sqlpp::table_t
template <template <typename...> class Table, typename Self, typename ...Args>
struct extract_table_members<std::tr2::__reflection_typelist<Table<Self, Args...>>> {
    // Now Args... are the types used in the base template
    using types = std::tuple<Args...>;
};

template <typename Table>
using extract_table_members_t = extract_table_members<Table>::types;

template<typename T>
auto launchAsync(T func) {
    return std::async(std::launch::async, func);
};

constexpr auto DEFAULT_CACHE_SIZE = 5;

constexpr const table::Years YEARS;
constexpr const table::Relationships RELATIONSHIPS;
constexpr const table::Countries COUNTRIES;
constexpr const table::Borders BORDERS;
constexpr const table::YearCities YEAR_CITIES;
constexpr const table::Cities CITIES;
constexpr const table::Events EVENTS;

template<typename Pool, typename Config, size_t cacheSize = DEFAULT_CACHE_SIZE>
class Persistence {
public:
    Persistence(std::shared_ptr<Config> config) :
        pool{config, cacheSize}
    {
        auto connection = pool.get();

        // initialize the database if necessary
        for (const auto& command : COMMANDS) {
            connection.execute(command);
        }
    }

    Data load(int year) {
        Data data{.year = year};

        if (const auto yearId = getYearId(year); yearId) {
            auto futureEvent = launchAsync([this, yearId = *yearId](){
                return this->getEvent(yearId);
            });

            for (auto [countryId, borderId] : getRelationship(yearId)) {
                auto futureCountryName = launchAsync([this, countryId](){
                    return this->getCountryName(countryId);
                });

                auto futureBorderContour = launchAsync([this, borderId](){
                    return this->getBorderContour(borderId);
                });

                if (auto [countryName, borderContour] = std::make_pair(futureBorderContour.get(), futureBorderContour.get()); 
                    countryName) {
                    data.countries.emplace_back(*countryName, borderContour);
                }
            }

            data.event = futureEvent.get();
        }

        return data;
    }

private:
    Pool pool;

    // less than 0 is BC
    std::optional<size_t> getYearId(int year) {
        for (const auto& row : pool.get()(sqlpp::select(YEARS.id).from(YEARS).where(YEARS.year == year))) {
            return row.id;
        }

        return std::nullopt;
    }

    std::vector<std::pair<size_t, size_t>> getRelationship(size_t yearId) {
        std::vector<std::pair<size_t, size_t>> countryAndBorderIds;

        for (const auto& row : pool.get()(sqlpp::select(RELATIONSHIPS.countryId, RELATIONSHIPS.borderId).
                                          from(RELATIONSHIPS).
                                          where(RELATIONSHIPS.yearId == yearId))) {
            countryAndBorderIds.emplace_back(row.countryId, row.borderId);
        }

        return countryAndBorderIds;
    }

    std::optional<std::string> getCountryName(size_t countryId) {
        for (const auto& row : pool.get()(sqlpp::select(COUNTRIES.name).from(COUNTRIES).where(COUNTRIES.id == countryId))) {
            return row.name;
        }

        return std::nullopt;
    }

    std::optional<size_t> getCountryId(std::string_view name)
    {
        for (const auto& row : pool.get()(sqlpp::select(COUNTRIES.id).from(COUNTRIES).where(COUNTRIES.name == name))) {
            return row.id;
        }

        return std::nullopt;
    }

    std::vector<Coordinate> getBorderContour(size_t borderId) {
        for (const auto& row : pool.get()(sqlpp::select(BORDERS.contour).from(BORDERS).where(BORDERS.id == borderId))) {
            return std::vector<Coordinate>{row.contour.blob, row.contour.blob + row.contour.len};
        }

        return {};
    }

    std::vector<int> getCityInfoIds(size_t yearId) {
        std::vector<int> ids;

        for (const auto& row : pool.get()(sqlpp::select(YEAR_CITIES.cityId).from(YEAR_CITIES).where(YEAR_CITIES.yearId == yearId))) {
            ids.emplace_back(row.cityInfoId);
        }

        return ids;
    }

    std::vector<City> getCities(const std::vector<int>& ids)
    {
        std::vector<City> cities;

        for (auto id : ids) {
            for (const auto& row : pool.get()(sqlpp::select(sqlpp::all_of(CITIES)).from(CITIES).where(CITIES.id == id))) {
                cities.emplace_back(row.name, Coordinate{row.latitude, row.longitude});
            }
        }
        
        return cities;
    }

    std::optional<City> getCity(std::string_view name)
    {
        for (const auto& row : pool.get()(sqlpp::select(CITIES.latitude, CITIES.longitude).from(CITIES).where(CITIES.name == name))) {
            return City{name, Coordinate{row.latitude, row.longitude}};
        }

        return std::nullopt;
    }

    std::optional<Event> getEvent(size_t yearId)
    {
        for (const auto& row : pool.get()(sqlpp::select(EVENTS.event).from(EVENTS).where(EVENTS.yearId == yearId))) {
            return Event{row.event};
        }
        
        return std::nullopt;
    }

    void upsert(int year)
    {
        if (auto yearId = getYearId(year); !yearId) {
            pool.get()(sqlpp::insert_into(YEARS).set(YEARS.year = year));
        }
    }

    void upsert(int yearId, const Event& event)
    {
        if (auto e = getEvent(yearId); e) {
            pool.get()(sqlpp::update(EVENTS).set(EVENTS.event = event.description).where(EVENTS.yearId == yearId));
        } else {
            pool.get()(sqlpp::insert_into(EVENTS).set(EVENTS.yearId = yearId, EVENTS.event = event.description));
        }
    }

    void upsert(int yearId, const City& city)
    {
        if (auto c = getCity(city.name); c && (*c != city)) {
            pool.get()(sqlpp::update(CITIES).set(CITIES.latitude = city.coordinate.latitude, CITIES.longitude = city.coordinate.longitude).where(CITIES.name == city.name));
        } else {
            pool.get()(sqlpp::insert_into(CITIES).set(CITIES.latitude = city.coordinate.latitude, CITIES.longitude = city.coordinate.longitude, CITIES.name = city.name));

            const auto& row = pool.get()(sqlpp::select(CITIES.id).from(CITIES).where(CITIES.name == city.name));
            const auto cityId = row.front().id;

            pool.get()(sqlpp::insert_into(YEAR_CITIES).set(YEAR_CITIES.cityId = cityId, YEAR_CITIES.yearId = yearId));
        }
    }


};
}

#endif
