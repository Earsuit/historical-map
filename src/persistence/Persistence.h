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

        connection.execute("PRAGMA foreign_keys = ON;");
    }

    Data load(int year) {
        Data data{.year = year};

        if (const auto& years = request<table::Years>(YEARS.year == year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            auto requestCountriesTask = std::async(std::launch::async, [this, yearId](){
                std::list<Country> countries;
                for (const auto& row : request<table::Relationships>(RELATIONSHIPS.yearId == yearId)) {
                    const auto countryName = this->request<table::Countries>(COUNTRIES.id == row.countryId, COUNTRIES.name).front().name;

                    const auto& borderRows = this->request<table::Borders>(BORDERS.id == row.borderId, BORDERS.contour);
                    const auto& borderContour = borderRows.front().contour;
                    const auto& contour = deserializeContour(Stream{borderContour.blob, borderContour.len});

                    countries.emplace_back(countryName, contour);
                }

                return countries;
            });

            auto requestCitiesTask = std::async(std::launch::async, [this, yearId](){
                std::list<City> cities;
                for (const auto& row : this->request<table::YearCities>(YEAR_CITIES.yearId == yearId, YEAR_CITIES.cityId)) {
                    const auto& cityRows = this->request<table::Cities>(CITIES.id == row.cityId);
                    const auto& city = cityRows.front();

                    cities.emplace_back(city.name, Coordinate{static_cast<float>(city.latitude), static_cast<float>(city.longitude)});
                }

                return cities;
            });

            auto requestEventTask = std::async(std::launch::async, [this, yearId]() -> std::optional<Event> {
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

        auto upsertCountriesTask = std::async(std::launch::deferred, [this, yearId, &countries = data.countries](){
            for (const auto& country : countries) {
                const auto stream = serializeContour<Stream>(country.borderContour);
                const auto contourHash = std::hash<std::string>{}(stream);
                const auto countryId = this->upsert<table::Countries>(COUNTRIES.name == country.name, COUNTRIES.name = country.name);
                bool borderNotExist = false;
                uint64_t borderId = 0;

                {
                    if (const auto& relationshipsRow = this->request<table::Relationships>(RELATIONSHIPS.yearId == yearId && RELATIONSHIPS.countryId == countryId,
                                                                                           RELATIONSHIPS.borderId);
                        relationshipsRow.empty()) {
                        borderNotExist = true;
                    } else {
                        borderNotExist = false;
                        borderId = relationshipsRow.front().borderId;
                    }
                }

                if (borderNotExist) {
                    // We don't use upsert for border because update contour is relative slow
                    if (const auto& borderRow = this->request<table::Borders>(BORDERS.hash == contourHash, BORDERS.id); borderRow.empty()) {
                        borderId = this->insert<table::Borders>(BORDERS.hash = contourHash, BORDERS.contour = std::vector<uint8_t>{stream});
                    } else {
                        borderId = borderRow.front().id;
                    }

                    this->insert<table::Relationships>(RELATIONSHIPS.yearId = yearId,
                                                       RELATIONSHIPS.countryId = countryId,
                                                       RELATIONSHIPS.borderId = borderId);
                } else {
                    // border id exists, check if we need to update
                    if (contourHash != static_cast<decltype(contourHash)>(this->request<table::Borders>(BORDERS.id == borderId, 
                                                                                                        BORDERS.hash).front().hash)) {
                        this->update<table::Borders>(BORDERS.id == borderId, 
                                                     BORDERS.hash = contourHash, 
                                                     BORDERS.contour = std::vector<uint8_t>{stream});
                    }
                }
            }
        });

        auto upsertCitiesTask = std::async(std::launch::deferred, [this, yearId, &cities = data.cities](){
            for (const auto& city : cities) {
                const auto cityId = this->upsert<table::Cities>(CITIES.name == city.name, 
                                                                CITIES.name = city.name, 
                                                                CITIES.latitude = city.coordinate.latitude,
                                                                CITIES.longitude = city.coordinate.longitude);

                this->upsert<table::YearCities>(YEAR_CITIES.yearId == yearId && YEAR_CITIES.cityId == cityId, 
                                                YEAR_CITIES.yearId = yearId, YEAR_CITIES.cityId = cityId);
            }
        });

        auto upsertEventTask = std::async(std::launch::deferred, [this, yearId, &event = data.event](){
            if (event) {
                this->upsert<table::Events>(EVENTS.yearId == yearId, EVENTS.yearId = yearId, EVENTS.event = event->description);
            }
        });

        upsertCountriesTask.wait();
        upsertCitiesTask.wait();
        upsertEventTask.wait();
    }

    void remove(const Data& data)
    {
        if (const auto& years = request<table::Years>(YEARS.year == data.year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            auto removeCountriesTask = std::async(std::launch::deferred, [this, yearId, &countries = data.countries](){
                for (const auto& country : countries) {
                    std::optional<uint64_t> countryId;
                    std::optional<uint64_t> borderId;
                    const auto stream  = serializeContour<Stream>(country.borderContour);
                    const auto contourHash = std::hash<std::string>{}(stream);

                    if (const auto rows = this->request<table::Countries>(COUNTRIES.name == country.name, COUNTRIES.id); !rows.empty()) {
                        countryId = rows.front().id;
                    }

                    if (const auto rows = this->request<table::Borders>(BORDERS.hash == contourHash, BORDERS.id); !rows.empty()) {
                        borderId = rows.front().id;
                    }

                    if (countryId && borderId) {
                        this->remove<table::Relationships>(RELATIONSHIPS.yearId == yearId &&
                                                           RELATIONSHIPS.countryId == *countryId &&
                                                           RELATIONSHIPS.borderId == *borderId);
                    }

                    if (countryId) {
                        if (const auto& rows = this->request<table::Relationships>(RELATIONSHIPS.countryId == *countryId, RELATIONSHIPS.id); rows.empty()) {
                            // we don't have any record of this country in the relationships table, so we can safely remove it from the countries table
                            this->remove<table::Countries>(COUNTRIES.id == *countryId);
                        }
                    }
                    
                    if (borderId) {
                        if (const auto& rows = this->request<table::Relationships>(RELATIONSHIPS.borderId == *borderId, RELATIONSHIPS.id); rows.empty()) {
                            // we don't have any record of this border in the relationships table, so we can safely remove it from the borders table
                            this->remove<table::Borders>(BORDERS.id == *borderId);
                        }
                    }
                }
            });

            auto removeCitiesTask = std::async(std::launch::deferred, [this, yearId, &cities = data.cities](){
                for (const auto& city : cities) {
                    if (const auto& rows = this->request<table::Cities>(CITIES.name == city.name); !rows.empty()) {
                        const auto cityId = rows.front().id;

                        this->remove<table::YearCities>(YEAR_CITIES.cityId == cityId && YEAR_CITIES.yearId == yearId);

                        if (const auto rows = this->request<table::YearCities>(YEAR_CITIES.cityId == cityId); rows.empty()) {
                            // we don't have any record of this city in the yearCities table, so we can safely remove it from the cities table
                            this->remove<table::Cities>(CITIES.id == cityId);
                        }
                    }
                }
            });
            
            if (data.event) {
                this->remove<table::Events>(EVENTS.event == data.event->description && EVENTS.yearId == yearId);
            }
            
            removeCountriesTask.wait();
            removeCitiesTask.wait();
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
    uint64_t upsert(Expression&& expression, Assignment&&... assignment)
    {
        uint64_t id = 0;
        // Here we can't forward the expression otherwise it will be moved inside the sqlpp,
        // and the expression used by the following update will have empty expression
        {
            // The returned object releases the connection after destruction
            const auto& ret = request<Table>(expression);
            if (ret.empty()) {
                return insert<Table>(std::forward<Assignment>(assignment)...);
            } else {
                id = ret.front().id;
            }
        }
        
        update<Table>(std::forward<Expression>(expression), std::forward<Assignment>(assignment)...);
        return id;
    }

    template<typename Table, typename Expression>
    auto remove(Expression&& expression)
    {
        constexpr const Table table;
        return pool.get()(sqlpp::remove_from(table).where(std::forward<Expression>(expression)));
    }
};
}

#endif
