#ifndef SRC_PERSISTENCE_PERSISTENCE_H
#define SRC_PERSISTENCE_PERSISTENCE_H

#include "src/persistence/Data.h"

// These are generated headers
#include "src/persistence/Commands.h"
#include "src/persistence/Table.h"

#include "sqlpp11/sqlpp11.h"

#include <memory>
#include <utility>
#include <tuple>
#include <vector>

namespace persistence {
constexpr const table::Years YEARS;
constexpr const table::Relationships RELATIONSHIPS;
constexpr const table::Countries COUNTRIES;
constexpr const table::Borders BORDERS;
constexpr const table::YearCities YEAR_CITIES;
constexpr const table::Cities CITIES;
constexpr const table::Notes NOTES;

template<typename Connection, typename Config>
class Persistence {
public:
    Persistence(std::shared_ptr<Config> config) :
        conn{config}
    {
        // initialize the database if necessary
        for (const auto& command : COMMANDS) {
            conn.execute(command);
        }

        conn.execute("PRAGMA foreign_keys = ON;");
    }

    Data load(int year) {
        Data data{.year = year};

        if (const auto& years = request<table::Years>(YEARS.year == year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            for (const auto& row : request<table::Relationships>(RELATIONSHIPS.yearId == yearId)) {
                const auto countryName = request<table::Countries>(COUNTRIES.id == row.countryId, COUNTRIES.name).front().name;

                const auto& borderRows = request<table::Borders>(BORDERS.id == row.borderId, BORDERS.contour);
                const auto& borderContour = borderRows.front().contour;
                const auto& contour = deserializeContour(Stream{borderContour.blob, borderContour.len});

                data.countries.emplace_back(countryName, contour);
            }

            for (const auto& row : request<table::YearCities>(YEAR_CITIES.yearId == yearId, YEAR_CITIES.cityId)) {
                const auto& cityRows = request<table::Cities>(CITIES.id == row.cityId);
                const auto& city = cityRows.front();

                data.cities.emplace_back(city.name, Coordinate{static_cast<float>(city.latitude), static_cast<float>(city.longitude)});
            }

            if (const auto& ret = request<table::Notes>(NOTES.yearId == yearId, NOTES.text); !ret.empty()) {
                data.note = Note{ret.front().text};
            }
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

        for (const auto& country : data.countries) {
            const auto stream = serializeContour<Stream>(country.borderContour);
            const auto contourHash = std::hash<std::string>{}(stream);
            const auto countryId = upsert<table::Countries>(COUNTRIES.name == country.name, COUNTRIES.name = country.name);
            bool borderNotExist = false;
            uint64_t borderId = 0;

            {
                if (const auto& relationshipsRow = request<table::Relationships>(RELATIONSHIPS.yearId == yearId && RELATIONSHIPS.countryId == countryId,
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
                if (const auto& borderRow = request<table::Borders>(BORDERS.hash == contourHash, BORDERS.id); borderRow.empty()) {
                    borderId = insert<table::Borders>(BORDERS.hash = contourHash, BORDERS.contour = std::vector<uint8_t>{stream});
                } else {
                    borderId = borderRow.front().id;
                }

                insert<table::Relationships>(RELATIONSHIPS.yearId = yearId,
                                             RELATIONSHIPS.countryId = countryId,
                                             RELATIONSHIPS.borderId = borderId);
            } else {
                // border id exists, check if we need to update
                if (contourHash != static_cast<decltype(contourHash)>(request<table::Borders>(BORDERS.id == borderId, 
                                                                                              BORDERS.hash).front().hash)) {
                    update<table::Borders>(BORDERS.id == borderId, 
                                           BORDERS.hash = contourHash, 
                                           BORDERS.contour = std::vector<uint8_t>{stream});
                }
            }
        }

        for (const auto& city : data.cities) {
            const auto cityId = upsert<table::Cities>(CITIES.name == city.name, 
                                                      CITIES.name = city.name, 
                                                      CITIES.latitude = city.coordinate.latitude,
                                                      CITIES.longitude = city.coordinate.longitude);

            upsert<table::YearCities>(YEAR_CITIES.yearId == yearId && YEAR_CITIES.cityId == cityId, 
                                      YEAR_CITIES.yearId = yearId, YEAR_CITIES.cityId = cityId);
        }

        if (data.note) {
            upsert<table::Notes>(NOTES.yearId == yearId, NOTES.yearId = yearId, NOTES.text = data.note->text);
        }
    }

    void remove(const Data& data)
    {
        if (const auto& years = request<table::Years>(YEARS.year == data.year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            for (const auto& country : data.countries) {
                std::optional<uint64_t> countryId;
                std::optional<uint64_t> borderId;
                const auto stream  = serializeContour<Stream>(country.borderContour);
                const auto contourHash = std::hash<std::string>{}(stream);

                if (const auto rows = request<table::Countries>(COUNTRIES.name == country.name, COUNTRIES.id); !rows.empty()) {
                    countryId = rows.front().id;
                }

                if (const auto rows = request<table::Borders>(BORDERS.hash == contourHash, BORDERS.id); !rows.empty()) {
                    borderId = rows.front().id;
                }

                if (countryId && borderId) {
                    remove<table::Relationships>(RELATIONSHIPS.yearId == yearId &&
                                                 RELATIONSHIPS.countryId == *countryId &&
                                                 RELATIONSHIPS.borderId == *borderId);
                }

                if (countryId) {
                    if (const auto& rows = request<table::Relationships>(RELATIONSHIPS.countryId == *countryId, RELATIONSHIPS.id); rows.empty()) {
                        // we don't have any record of this country in the relationships table, so we can safely remove it from the countries table
                        remove<table::Countries>(COUNTRIES.id == *countryId);
                    }
                }
                
                if (borderId) {
                    if (const auto& rows = request<table::Relationships>(RELATIONSHIPS.borderId == *borderId, RELATIONSHIPS.id); rows.empty()) {
                        // we don't have any record of this border in the relationships table, so we can safely remove it from the borders table
                        remove<table::Borders>(BORDERS.id == *borderId);
                    }
                }
            }

            for (const auto& city : data.cities) {
                if (const auto& rows = request<table::Cities>(CITIES.name == city.name); !rows.empty()) {
                    const auto cityId = rows.front().id;

                    remove<table::YearCities>(YEAR_CITIES.cityId == cityId && YEAR_CITIES.yearId == yearId);

                    if (const auto rows = request<table::YearCities>(YEAR_CITIES.cityId == cityId); rows.empty()) {
                        // we don't have any record of this city in the yearCities table, so we can safely remove it from the cities table
                        remove<table::Cities>(CITIES.id == cityId);
                    }
                }
            }
            
            if (data.note) {
                remove<table::Notes>(NOTES.text == data.note->text && NOTES.yearId == yearId);
            }
        }
    }

private:
    Connection conn;

    template<typename Table>
    auto request()
    {
        constexpr const Table table;
        return conn(sqlpp::select(sqlpp::all_of(table)).from(table).unconditionally());
    }

    template<typename Table, typename Expression, typename... Column>
    auto request(Expression&& expression, Column&&... column)
    {
        constexpr const Table table;
        return conn(sqlpp::select(std::forward<Column>(column)...).from(table).where(std::forward<Expression>(expression)));
    }

    template<typename Table, typename Expression>
    auto request(Expression&& expression)
    {
        constexpr const Table table;
        return conn(sqlpp::select(sqlpp::all_of(table)).from(table).where(std::forward<Expression>(expression)));
    }
 
    template<typename Table, typename... Assignment>
    uint64_t insert(Assignment&&... assignment)
    {
        constexpr const Table table;
        return conn(sqlpp::insert_into(table).set(std::make_tuple(std::forward<Assignment>(assignment)...)));
    }

    template<typename Table, typename Expression, typename... Assignment>
    void update(Expression&& expression, Assignment&&... assignment)
    {
        constexpr const Table table;
        conn(sqlpp::update(table).set(std::make_tuple(std::forward<Assignment>(assignment)...)).where(std::forward<Expression>(expression)));
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
        return conn(sqlpp::remove_from(table).where(std::forward<Expression>(expression)));
    }
};
}

#endif
