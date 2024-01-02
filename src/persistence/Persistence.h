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
#include <tuple>

namespace persistence {

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

        if (const auto& years = request<table::Years>(YEARS.year == year); !years.empty()) {
            const auto yearId = years.front().id;

            auto requestCountriesTask = launchAsync([this, yearId](){
                std::vector<Country> countries;
                for (const auto& row : request<table::Relationships>(RELATIONSHIPS.yearId == yearId)) {
                    const auto& countryName = this->request<table::Countries>(COUNTRIES.id == row.countryId).front().name;
                    const auto& borderContour = this->request<table::Borders>(BORDERS.id == row.borderId).front().contour;

                    countries.emplace_back(countryName, {borderContour.blob, borderContour.blob + borderContour.len});
                }

                return countries;
            });

            auto requestCitiesTask = launchAsync([this, yearId](){
                std::vector<City> cities;
                for (const auto& row : this->request<table::YearCities>(YEAR_CITIES.yearId == yearId)) {
                    const auto& city = this->request<table::Cities>(CITIES.id == row.cityId).front();

                    cities.emplace_back(city.name, Coordinate{city.latitude, city.longitude});
                }

                return cities;
            });

            auto requestEventTask = launchAsync([this, yearId]() -> std::optional<Event> {
                if (const auto& ret = this->request<table::Events>(EVENTS.yearId == yearId); !ret.empty()) {
                    return Event{ret.front().event};
                } else {
                    return std::nullopt;
                }
            });

            data.countries = requestCountriesTask.wait();
            data.cities = requestCitiesTask.get();
            data.event = requestEventTask.get();
        }

        return data;
    }

    void upsert(const Data& data)
    {
        if (const auto& years = request<table::Years>(YEARS.year == data.year); !years.empty()) {
            const auto yearId = years.front().id;

            auto upsertCountriesTask = launchAsync([this, yearId, &countries = data.countries](){
                for (const auto& country : countries) {
                    this->upsert<table::Countries>(COUNTRIES.name == country.name, COUNTRIES.name = country.name);
                    this->upsert<table::Borders>(BORDERS.contour == country.borderContour, BORDERS.contour = country.borderContour);

                    const auto countryId = this->request<table::Countries>(COUNTRIES.name == country.name).front().id;
                    const auto borderId = this->request<table::Borders>(BORDERS.contour == country.borderContour).front().id;
                    
                    this->upsert<table::Relationships>(RELATIONSHIPS.yearId == yearId && RELATIONSHIPS.countryId == countryId && RELATIONSHIPS.borderId == borderId,
                                                       RELATIONSHIPS.yearId = yearId,
                                                       RELATIONSHIPS.countryId = countryId,
                                                       RELATIONSHIPS.borderId = borderId);
                }
            });

            auto upsertCitiesTask = launchAsync([this, yearId, &cities = data.cities](){
                for (const auto& city : cities) {
                    this->upsert<table::Cities>(CITIES.name == city.name, 
                                                CITIES.name = city.name, 
                                                CITIES.latitude = city.coordinate.latitude,
                                                CITIES.longitude = city.coordinate.longitude);

                    const auto cityId = this->request<table::Cities>(CITIES.name == city.name).front().id;

                    this->upsert<table::YearCities>(YEAR_CITIES.yearId == yearId && YEAR_CITIES.cityId == cityId, 
                                                    YEAR_CITIES.cityId = cityId);
                }
            });

            auto upsertEventTask = launchAsync([this, yearId, &event = data.event](){
                if (event) {
                    this->upsert<table::Events>(EVENTS.yearId == yearId, EVENTS.event = (*event).description);
                }
            });
        }
    }

private:
    Pool pool;

    template<typename Table>
    auto request()
    {
        constexpr const Table table;
        return pool.get()(sqlpp::select(sqlpp::all_of(table)).from(table).unconditionally());
    }

    template<typename Table, typename Expression>
    auto request(Expression&& expression)
    {
        constexpr const Table table;
        return pool.get()(sqlpp::select(sqlpp::all_of(table)).from(table).where(std::forward<Expression>(expression)));
    }

    template<typename Table, typename... Assignments>
    void insert(Assignments... assignments)
    {
        constexpr const Table table;
        pool.get()(sqlpp::insert_into(table).set(std::make_tuple(assignments...)));
    }

    template<typename Table, typename Expression, typename... Assignments>
    void update(Expression&& expression, Assignments... assignments)
    {
        constexpr const Table table;
        pool.get()(sqlpp::update(table).set(std::make_tuple(assignments...)).where(std::forward<Expression>(expression)));
    }

    template<typename Table, typename Expression, typename... Assignments>
    void upsert(Expression&& expression, Assignments... assignments)
    {
        constexpr const Table table;
        const auto& ret = request<Table>(std::forward<Expression>(expression));
        if (ret.empty()) {
            insert<Table>(std::forward<Assignments>(assignments)...);
        } else {
            update<Table>(std::forward<Expression>(expression), std::forward<Assignments>(assignments)...);
        }
    }
};
}

#endif
