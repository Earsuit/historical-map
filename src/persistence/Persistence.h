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
#include <vector>

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

        if (const auto& years = request<table::Years>(YEARS.year == year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            auto requestCountriesTask = launchAsync([this, yearId](){
                std::vector<Country> countries;
                for (const auto& row : request<table::Relationships>(RELATIONSHIPS.yearId == yearId)) {
                    const auto countryName = this->request<table::Countries>(COUNTRIES.id == row.countryId, COUNTRIES.name).front().name;

                    const auto& borderRows = this->request<table::Borders>(BORDERS.id == row.borderId, BORDERS.contour);
                    const auto& borderContour = borderRows.front().contour;
                    const auto& contour = deserializeContour(std::vector<uint8_t>{borderContour.blob, borderContour.blob + borderContour.len});

                    countries.emplace_back(countryName, contour);
                }

                return countries;
            });

            auto requestCitiesTask = launchAsync([this, yearId](){
                std::vector<City> cities;
                for (const auto& row : this->request<table::YearCities>(YEAR_CITIES.yearId == yearId, YEAR_CITIES.cityId)) {
                    const auto& cityRows = this->request<table::Cities>(CITIES.id == row.cityId);
                    const auto& city = cityRows.front();

                    cities.emplace_back(city.name, Coordinate{city.latitude, city.longitude});
                }

                return cities;
            });

            auto requestEventTask = launchAsync([this, yearId]() -> std::optional<Event> {
                if (const auto& ret = this->request<table::Events>(EVENTS.yearId == yearId, EVENTS.event); !ret.empty()) {
                    return Event{ret.front().event};
                } else {
                    return std::nullopt;
                }
            });

            data.countries = requestCountriesTask.get();
            data.cities = requestCitiesTask.get();
            data.event = requestEventTask.get();
        }

        return data;
    }

    void upsert(const Data& data)
    {
        uint64_t yearId = 0;
        if (const auto& years = request<table::Years>(YEARS.year == data.year, YEARS.id); !years.empty()) {
            yearId = years.front().id;
        } else {
            yearId = insert<table::Years>(YEARS.year = data.year);
        }

        auto upsertCountriesTask = launchAsync([this, yearId, &countries = data.countries](){
            for (const auto& country : countries) {
                const auto& serializedContour = serializeContour(country.borderContour);

                if (const auto& countryIdOpt = this->upsert<table::Countries>(COUNTRIES.name == country.name, COUNTRIES.name = country.name); countryIdOpt) {
                    // country doesn't exist in the table before, it means the contour doesn't exist either
                    const auto borderId = this->insert<table::Borders>(BORDERS.contour = serializedContour);

                    this->insert<table::Relationships>(RELATIONSHIPS.yearId = yearId, RELATIONSHIPS.countryId = *countryIdOpt, RELATIONSHIPS.borderId = borderId);
                } else {
                    // country already exists in the table, so as the border
                    const uint64_t countryId = this->request<table::Countries>(COUNTRIES.name == country.name, COUNTRIES.id).front().id;
                    const uint64_t borderId = this->request<table::Relationships>(RELATIONSHIPS.yearId == yearId && RELATIONSHIPS.countryId == countryId, RELATIONSHIPS.borderId).front().borderId;

                    this->update<table::Borders>(BORDERS.id == borderId, BORDERS.contour = serializedContour);
                }
            }
        });

        auto upsertCitiesTask = launchAsync([this, yearId, &cities = data.cities](){
            for (const auto& city : cities) {
                uint64_t cityId = 0;
                if (const auto cityIdOpt = this->upsert<table::Cities>(CITIES.name == city.name, 
                                                                    CITIES.name = city.name, 
                                                                    CITIES.latitude = city.coordinate.latitude,
                                                                    CITIES.longitude = city.coordinate.longitude); cityIdOpt) {
                    cityId = *cityIdOpt;
                } else {
                    cityId = this->request<table::Cities>(CITIES.name == city.name, CITIES.id).front().id;
                }

                this->upsert<table::YearCities>(YEAR_CITIES.yearId == yearId && YEAR_CITIES.cityId == cityId, 
                                                YEAR_CITIES.yearId = yearId, YEAR_CITIES.cityId = cityId);
            }
        });

        auto upsertEventTask = launchAsync([this, yearId, &event = data.event](){
            if (event) {
                this->upsert<table::Events>(EVENTS.yearId == yearId, EVENTS.yearId = yearId, EVENTS.event = event->description);
            }
        });

        upsertCountriesTask.wait();
        upsertCitiesTask.wait();
        upsertEventTask.wait();
    }

private:
    Pool pool;

    template<typename Table>
    auto request()
    {
        constexpr const Table table;
        return pool.get()(sqlpp::select(sqlpp::all_of(table)).from(table).unconditionally());
    }

    template<typename Table, typename Expression, typename... Column>
    auto request(Expression&& expression, Column&&... column)
    {
        constexpr const Table table;
        return pool.get()(sqlpp::select(std::forward<Column>(column)...).from(table).where(std::forward<Expression>(expression)));
    }

    template<typename Table, typename Expression>
    auto request(Expression&& expression)
    {
        constexpr const Table table;
        return pool.get()(sqlpp::select(sqlpp::all_of(table)).from(table).where(std::forward<Expression>(expression)));
    }
 
    template<typename Table, typename... Assignment>
    uint64_t insert(Assignment&&... assignment)
    {
        constexpr const Table table;
        return pool.get()(sqlpp::insert_into(table).set(std::make_tuple(std::forward<Assignment>(assignment)...)));
    }

    template<typename Table, typename Expression, typename... Assignment>
    void update(Expression&& expression, Assignment&&... assignment)
    {
        constexpr const Table table;
        pool.get()(sqlpp::update(table).set(std::make_tuple(std::forward<Assignment>(assignment)...)).where(std::forward<Expression>(expression)));
    }

    template<typename Table, typename Expression, typename... Assignment>
    std::optional<uint64_t> upsert(Expression&& expression, Assignment&&... assignment)
    {
        constexpr const Table table;
        // Here we can't forward the expression otherwise it will be moved inside the sqlpp,
        // and the expression used by the following update will have empty expression
        const auto& ret = request<Table>(expression);
        if (ret.empty()) {
            return insert<Table>(std::forward<Assignment>(assignment)...);
        } else {
            update<Table>(std::forward<Expression>(expression), std::forward<Assignment>(assignment)...);
            return std::nullopt;
        }
    }
};
}

#endif
