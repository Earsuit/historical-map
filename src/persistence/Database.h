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
#include <optional>

namespace persistence {
constexpr const table::Years YEARS;
constexpr const table::YearCountries YEAR_COUNTRIES;
constexpr const table::Countries COUNTRIES;
constexpr const table::Borders BORDERS;
constexpr const table::YearCities YEAR_CITIES;
constexpr const table::Cities CITIES;
constexpr const table::Notes NOTES;
constexpr const table::YearNotes YEAR_NOTES;

template<typename Connection, typename Config>
class Database {
public:
    Database(std::shared_ptr<Config> config) :
        conn{config}
    {
        // initialize the database if necessary
        for (const auto& command : COMMANDS) {
            conn.execute(command);
        }

        conn.execute("PRAGMA foreign_keys = ON;");
    }

    std::vector<std::string> loadCountryList(int year) 
    {
        std::vector<std::string> countries;

        if (const auto& years = request<table::Years>(YEARS.year == year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            for (const auto& row : request<table::YearCountries>(YEAR_COUNTRIES.yearId == yearId)) {
                const auto countryName = request<table::Countries>(COUNTRIES.id == row.countryId, COUNTRIES.name).front().name;

                countries.emplace_back(countryName);
            }
        }

        return countries;
    }

    std::vector<std::string> loadCityList(int year)
    {
        std::vector<std::string> cities;

        if (const auto& years = request<table::Years>(YEARS.year == year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            for (const auto& row : request<table::YearCities>(YEAR_CITIES.yearId == yearId, YEAR_CITIES.cityId)) {
                const auto& cityRows = request<table::Cities>(CITIES.id == row.cityId);
                const auto& city = cityRows.front();

                cities.emplace_back(city.name);
            }
        }

        return cities;
    }

    Note loadNote(int year)
    {
        Note note;

        if (const auto& years = request<table::Years>(YEARS.year == year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            if (const auto& ret = request<table::YearNotes>(YEAR_NOTES.yearId == yearId, YEAR_NOTES.noteId); !ret.empty()) {
                note.text = request<table::Notes>(NOTES.id == ret.front().noteId, NOTES.text).front().text;
            }
        }

        return note;
    }

    std::optional<Country> loadCountry(int year, const std::string& name)
    {
        if (const auto& years = request<table::Years>(YEARS.year == year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            if (const auto& country = request<table::Countries>(COUNTRIES.name == name, COUNTRIES.id); !country.empty()) {
                const auto countryId = country.front().id;

                const auto borderId = request<table::YearCountries>(YEAR_COUNTRIES.yearId == yearId && YEAR_COUNTRIES.countryId == countryId,
                                                                  YEAR_COUNTRIES.borderId).front().borderId;
                const auto& borderRows = request<table::Borders>(BORDERS.id == borderId, BORDERS.contour);
                const auto& borderContour = borderRows.front().contour;
                const auto& contour = deserializeContour(Stream{borderContour.blob, borderContour.len});

                return Country{name, contour};
            }
        }

        return std::nullopt;
    }

    std::optional<City> loadCity(int year, const std::string& name)
    {
        if (const auto& years = request<table::Years>(YEARS.year == year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            if (const auto& ret = request<table::Cities>(CITIES.name == name); !ret.empty()) {
                const auto& city = ret.front();

                if (const auto& ret = request<table::YearCities>(YEAR_CITIES.yearId == yearId && YEAR_CITIES.cityId == city.id); !ret.empty()) {
                    return City{city.name, Coordinate{static_cast<float>(city.latitude), static_cast<float>(city.longitude)}};
                }
            }
        }

        return std::nullopt;
    }

    Data load(int year) 
    {
        Data data{.year = year};

        if (const auto& years = request<table::Years>(YEARS.year == year, YEARS.id); !years.empty()) {
            const auto yearId = years.front().id;

            for (const auto& row : request<table::YearCountries>(YEAR_COUNTRIES.yearId == yearId)) {
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

            if (const auto& ret = request<table::YearNotes>(YEAR_NOTES.yearId == yearId, YEAR_NOTES.noteId); !ret.empty()) {
                data.note.text = request<table::Notes>(NOTES.id == ret.front().noteId, NOTES.text).front().text;
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
            uint64_t borderId = 0;

            if (const auto& relationshipsRows = request<table::YearCountries>(YEAR_COUNTRIES.yearId == yearId && YEAR_COUNTRIES.countryId == countryId,
                                                                              YEAR_COUNTRIES.borderId, YEAR_COUNTRIES.id);
                                                                              relationshipsRows.empty()) {
                // We don't use upsert for border because update contour is relative slow
                if (const auto& borderRow = request<table::Borders>(BORDERS.hash == contourHash, BORDERS.id); borderRow.empty()) {
                    borderId = insert<table::Borders>(BORDERS.hash = contourHash, BORDERS.contour = std::vector<uint8_t>{stream});
                } else {
                    borderId = borderRow.front().id;
                }

                insert<table::YearCountries>(YEAR_COUNTRIES.yearId = yearId,
                                                    YEAR_COUNTRIES.countryId = countryId,
                                                    YEAR_COUNTRIES.borderId = borderId);
            } else {
                // The same country in this year already exists, check if the border need to be updated
                borderId = relationshipsRows.front().borderId;
                if (contourHash != static_cast<decltype(contourHash)>(request<table::Borders>(BORDERS.id == borderId, 
                                                                                              BORDERS.hash).front().hash)) {
                    if (const auto& boardUsedByOthers = request<table::YearCountries>(YEAR_COUNTRIES.borderId == borderId && 
                                                                                      YEAR_COUNTRIES.id != relationshipsRows.front().id,
                                                                                      YEAR_COUNTRIES.borderId);
                                                                                      boardUsedByOthers.empty()) {
                        // The existing contour is not used on other places, delete it first
                        remove<table::Borders>(BORDERS.id == borderId);
                        // then create a new one
                        borderId = upsert<table::Borders>(BORDERS.hash == contourHash, 
                                                          BORDERS.hash = contourHash, 
                                                          BORDERS.contour = std::vector<uint8_t>{stream});
                        // and insert to the relationship table because the previous relationship is deleted due to ON DELETE CASCADE
                        insert<table::YearCountries>(YEAR_COUNTRIES.borderId = borderId, 
                                                     YEAR_COUNTRIES.yearId = yearId,
                                                     YEAR_COUNTRIES.countryId = countryId);
                    } else {
                        // This border is used on other places, we can't update it because it will affect the countries use it
                        // We create a new one instead if this border doesn't exist
                        borderId = upsert<table::Borders>(BORDERS.hash == contourHash, 
                                                          BORDERS.hash = contourHash, 
                                                          BORDERS.contour = std::vector<uint8_t>{stream});
                        update<table::YearCountries>(YEAR_COUNTRIES.id == relationshipsRows.front().id,
                                                     YEAR_COUNTRIES.borderId = borderId);
                    }
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

        // note
        const auto hashedText = std::hash<std::string>{}(data.note.text);
        // first search if the text exists
        int noteId = 0;
        if (const auto& notesRows = request<table::Notes>(NOTES.hash == hashedText, NOTES.id); notesRows.empty()) {
            // no same text found, insert it
            noteId = insert<table::Notes>(NOTES.hash = hashedText, NOTES.text = data.note.text);
        } else {
            // same text found, use its id
            noteId = notesRows.front().id;
        }

        if (const auto& yearNotesRows = request<table::YearNotes>(YEAR_NOTES.yearId == yearId, YEAR_NOTES.noteId); yearNotesRows.empty()) {
            // This year doesn't have note, 
            insert<table::YearNotes>(YEAR_NOTES.yearId = yearId, YEAR_NOTES.noteId = noteId);
        } else {
            // This year already has note, check if it is the same
            const size_t noteIdFromTable = yearNotesRows.front().noteId;
            if (noteId != noteIdFromTable) {
                update<table::YearNotes>(YEAR_NOTES.yearId == yearId, YEAR_NOTES.noteId = noteId);
                if (const auto& ret = request<table::YearNotes>(YEAR_NOTES.noteId == noteIdFromTable); ret.empty()) {
                    // the noteId is not used in other years, we are safe to delete the old one after update the noteId in the yearNotes table
                    remove<table::Notes>(NOTES.id == noteIdFromTable);
                }
            }
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
                    remove<table::YearCountries>(YEAR_COUNTRIES.yearId == yearId &&
                                                 YEAR_COUNTRIES.countryId == *countryId &&
                                                 YEAR_COUNTRIES.borderId == *borderId);
                }

                if (countryId) {
                    if (const auto& rows = request<table::YearCountries>(YEAR_COUNTRIES.countryId == *countryId, YEAR_COUNTRIES.id); rows.empty()) {
                        // we don't have any record of this country in the relationships table, so we can safely remove it from the countries table
                        remove<table::Countries>(COUNTRIES.id == *countryId);
                    }
                }
                
                if (borderId) {
                    if (const auto& rows = request<table::YearCountries>(YEAR_COUNTRIES.borderId == *borderId, YEAR_COUNTRIES.id); rows.empty()) {
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
            
            // note
            const auto hashedText = std::hash<std::string>{}(data.note.text);
            if (const auto& notesRows = request<table::Notes>(NOTES.hash == hashedText, NOTES.id); !notesRows.empty()) {
                const auto noteId = notesRows.front().id;

                remove<table::YearNotes>(YEAR_NOTES.noteId == noteId && YEAR_NOTES.yearId == yearId);

                if (const auto& yearNotesRows = request<table::YearNotes>(YEAR_NOTES.noteId == noteId && YEAR_NOTES.yearId != yearId); yearNotesRows.empty()) {
                    // this note is not used in other years, it is safe to delete it
                    remove<table::Notes>(NOTES.hash == hashedText);
                }
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
