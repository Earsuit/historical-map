#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"

#include "src/persistence/Database.h"

#include <gtest/gtest.h>
#include <cstdio>

namespace {
using namespace sqlpp::sqlite3;

// https://www.sqlite.org/inmemorydb.html
// in memory database from two connections
constexpr auto DATABASE_NAME = "file:memdb1?mode=memory";

class DatabaseTest : public ::testing::Test {
public:
    DatabaseTest():
        database{config},
        monitor{config}
    {
    }

    ~DatabaseTest()
    {
        // clear the in-memory to remove the cache in Windows
        monitor.execute("DELETE FROM yearCountries");
        monitor.execute("DELETE FROM yearCities");
        monitor.execute("DELETE FROM cities");
        monitor.execute("DELETE FROM yearNotes");
        monitor.execute("DELETE FROM notes");
        monitor.execute("DELETE FROM years");
        monitor.execute("DELETE FROM countries");
        monitor.execute("DELETE FROM borders");
    }

    std::shared_ptr<connection_config> config = std::make_shared<connection_config>(DATABASE_NAME, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,  "", true);
    persistence::Database<connection, connection_config> database;
    connection monitor;
};

TEST_F(DatabaseTest, InsertEmpty)
{
    const persistence::Data data{1900};

    database.upsert(data);

    EXPECT_EQ(database.load(1900), data);
}

TEST_F(DatabaseTest, RemoveEmpty)
{
    const persistence::Data data{1900};

    database.remove(data);

    EXPECT_EQ(database.load(1900), data);
}

TEST_F(DatabaseTest, LoadNonExistYear)
{
    EXPECT_EQ(database.load(2000), persistence::Data{2000});
}

TEST_F(DatabaseTest, InsertOneCountry)
{
    const persistence::Country country{"TestCountry", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Data data{1900, {country}};

    database.upsert(data);

    EXPECT_EQ(database.load(1900), data);
}

TEST_F(DatabaseTest, InsertDuplicateCountries)
{
    const persistence::Country country{"TestCountry", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Data data{1900, {country}};

    database.upsert(data);
    database.upsert(data);

    EXPECT_EQ(database.load(1900), data);
    
    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(DatabaseTest, InsertTwoCountriesDifferentYear)
{
    int year1 = 1900;
    const persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Data data1{year1, {country1}};

    int year2 = 2000;
    const persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::Data data2{year2, {country2}};

    database.upsert(data1);
    database.upsert(data2);

    EXPECT_EQ(database.load(year1), data1);
    EXPECT_EQ(database.load(year2), data2);
}

TEST_F(DatabaseTest, InsertTwoCountriesSameYearSeparately)
{
    int year = 1900;
    const persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Data data1{year, {country1}};

    const persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::Data data2{year, {country2}};

    database.upsert(data1);
    database.upsert(data2);
    
    const persistence::Data expect{year, {country1, country2}};
    EXPECT_EQ(database.load(year), expect);
}

TEST_F(DatabaseTest, InsertTwoCountriesSameYearTogether)
{
    int year = 1900;
    const persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::Data data{year, {country1, country2}};

    database.upsert(data);

    EXPECT_EQ(database.load(year), data);
}

TEST_F(DatabaseTest, UpdateOneOfTheCountry)
{
    int year = 1900;
    const persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::Country updateCountry2{"Two", {persistence::Coordinate{9,10}, persistence::Coordinate{11,12}}};
    persistence::Data data{year, {country1, country2}};

    database.upsert(data);

    data.countries = std::list<persistence::Country>{updateCountry2};

    database.upsert(data);

    const persistence::Data expect{year, {country1, updateCountry2}};

    EXPECT_EQ(database.load(year), expect);

    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 2);
}

TEST_F(DatabaseTest, InsertTwoCountriesWithSameContourAtDifferentYear)
{
    int year1 = 1900;
    int year2 = 2000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data data1{year1, {country1}};
    persistence::Data data2{year2, {country2}};

    database.upsert(data1);
    database.upsert(data2);

    EXPECT_EQ(database.load(year1), data1);
    EXPECT_EQ(database.load(year2), data2);

    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(DatabaseTest, RemoveOneCountryDifferentBorder)
{
    int year = 1900;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    persistence::Data data{year, {country1, country2}};
    persistence::Data remove{year, {country2}};
    persistence::Data expect{year, {country1}};

    database.upsert(data);

    database.remove(remove);

    EXPECT_EQ(database.load(year), expect);

    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(DatabaseTest, RemoveOneCountrySameBorderSameName)
{
    int year1 = 1900;
    int year2 = 2000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data remove{year2, {country2}};
    persistence::Data expect{year1, {country1}};

    database.upsert(persistence::Data{year1, {country1}});
    database.upsert(persistence::Data{year2, {country2}});

    database.remove(remove);

    EXPECT_EQ(database.load(year1), expect);
}

TEST_F(DatabaseTest, RemoveOneCountrySameBorderDifferentName)
{
    int year1 = 1900;
    int year2 = 2000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data remove{year2, {country2}};
    persistence::Data expect{year1, {country1}};

    database.upsert(persistence::Data{year1, {country1}});
    database.upsert(persistence::Data{year2, {country2}});

    database.remove(remove);

    EXPECT_EQ(database.load(year1), expect);
}

TEST_F(DatabaseTest, RemoveOneCountryFromWrongYear)
{
    int year1 = 1900;
    int year2 = 2000;
    int year3 = 3000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data remove{year3, {country2}};
    persistence::Data expect1{year1, {country1}};
    persistence::Data expect2{year2, {country2}};

    database.upsert(persistence::Data{year1, {country1}});
    database.upsert(persistence::Data{year2, {country2}});

    database.remove(remove);

    EXPECT_EQ(database.load(year1), expect1);
    EXPECT_EQ(database.load(year2), expect2);
}

TEST_F(DatabaseTest, RemoveAllCountries)
{
    int year = 1900;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data remove{year, {country1, country2}};
    persistence::Data expect{year};

    database.upsert(persistence::Data{year, {country1, country2}});

    database.remove(remove);

    EXPECT_EQ(database.load(year), expect);

    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 0);
}

TEST_F(DatabaseTest, UpdateBorderUsedByMultipleYears)
{
    int year1 = 1900;
    int year2 = 1901;
    persistence::Country country{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country update{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}, persistence::Coordinate{5,6}}};
    const persistence::Data expectedYear1{year1, {update}};
    const persistence::Data expectedYear2{year2, {country}};

    database.upsert(persistence::Data{year1, {country}});
    database.upsert(persistence::Data{year2, {country}});

    database.upsert(persistence::Data{year1, {update}});

    EXPECT_EQ(database.load(year1), expectedYear1);
    EXPECT_EQ(database.load(year2), expectedYear2);
}

TEST_F(DatabaseTest, UpdateBorderForMultipleYearsUsedByMultipleYears)
{
    persistence::Country country{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country update{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}, persistence::Coordinate{5,6}}};
    const persistence::Data expectedYear1{1900, {update}};
    const persistence::Data expectedYear2{1901, {update}};
    const persistence::Data expectedYear3{1902, {country}};
    const persistence::Data expectedYear4{1903, {country}};

    database.upsert(persistence::Data{1900, {country}});
    database.upsert(persistence::Data{1901, {country}});
    database.upsert(persistence::Data{1902, {country}});
    database.upsert(persistence::Data{1903, {country}});

    database.upsert(persistence::Data{1900, {update}});
    database.upsert(persistence::Data{1901, {update}});

    EXPECT_EQ(database.load(1900), expectedYear1);
    EXPECT_EQ(database.load(1901), expectedYear2);
    EXPECT_EQ(database.load(1902), expectedYear3);
    EXPECT_EQ(database.load(1903), expectedYear4);
}

TEST_F(DatabaseTest, UpdateBorderMultipleTimes)
{
    persistence::Country country{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country update{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}, persistence::Coordinate{5,6}}};
    const persistence::Data expectedYear1{1900, {country}};
    const persistence::Data expectedYear2{1901, {country}};
    const persistence::Data expectedYear3{1902, {country}};
    const persistence::Data expectedYear4{1903, {country}};

    database.upsert(persistence::Data{1900, {country}});
    database.upsert(persistence::Data{1901, {country}});
    database.upsert(persistence::Data{1902, {country}});
    database.upsert(persistence::Data{1903, {country}});

    database.upsert(persistence::Data{1900, {update}});
    database.upsert(persistence::Data{1901, {update}});

    database.upsert(persistence::Data{1900, {country}});
    database.upsert(persistence::Data{1901, {country}});

    EXPECT_EQ(database.load(1900), expectedYear1);
    EXPECT_EQ(database.load(1901), expectedYear2);
    EXPECT_EQ(database.load(1902), expectedYear3);
    EXPECT_EQ(database.load(1903), expectedYear4);
}

TEST_F(DatabaseTest, UpdateBorderUsedByMultipleCountries)
{
    int year = 1900;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country update{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}, persistence::Coordinate{5,6}}};
    const persistence::Data expectedYear{year, {country1, update}};

    database.upsert(persistence::Data{year, {country1, country2}});
    database.upsert(persistence::Data{year, {update}});

    EXPECT_EQ(database.load(year), expectedYear);
}

TEST_F(DatabaseTest, InsertOneCity)
{
    int year = 1900;
    persistence::Data data{year};
    data.cities.emplace_back("One", persistence::Coordinate{1, 2});

    database.upsert(data);

    EXPECT_EQ(database.load(year), data);
}

TEST_F(DatabaseTest, InsertDuplicateCities)
{
    int year = 1900;
    persistence::Data data{year};
    data.cities.emplace_back("One", persistence::Coordinate{1, 2});

    database.upsert(data);
    database.upsert(data);

    EXPECT_EQ(database.load(year), data);
}

TEST_F(DatabaseTest, InsertOneCountryWithDifferentContourAtDifferentYear)
{
    int year1 = 1900;
    int year2 = 2000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"One", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    persistence::Data data1{year1, {country1}};
    persistence::Data data2{year2, {country2}};

    database.upsert(data1);
    database.upsert(data2);

    EXPECT_EQ(database.load(year1), data1);
    EXPECT_EQ(database.load(year2), data2);
}

TEST_F(DatabaseTest, InsertTwoCitiesSameYearSeparately)
{
    int year = 1900;
    persistence::Data data1{year}, data2{year};
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    data1.cities.emplace_back(city1);
    data2.cities.emplace_back(city2);

    database.upsert(data1);
    database.upsert(data2);

    const persistence::Data expect{year, {}, {city1, city2}};
    EXPECT_EQ(database.load(year), expect);
}

TEST_F(DatabaseTest, InsertOneCityDifferentCoordDifferntYear)
{
    int year1 = 1900;
    int year2 = 2000;
    persistence::Data data1{year1}, data2{year2};
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"One", {3, 4}};
    data1.cities.emplace_back(city1);
    data2.cities.emplace_back(city2);

    database.upsert(data1);
    database.upsert(data2);

    const persistence::Data expect1{year1, {}, {city2}};
    EXPECT_EQ(database.load(year1), expect1);

    const persistence::Data expect2{year2, {}, {city2}};
    EXPECT_EQ(database.load(year2), expect2);
}

TEST_F(DatabaseTest, InsertTwoCitiesSameYearTogether)
{
    int year = 1900;
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    const persistence::Data data{year, {}, {city1, city2}};

    database.upsert(data);

    EXPECT_EQ(database.load(year), data);
}

TEST_F(DatabaseTest, UpdateOneOfTheCity)
{
    int year = 1900;
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    const persistence::City updateCity2{"Two", {5,6}};
    persistence::Data data{year, {}, {city1, city2}};

    database.upsert(data);

    data.cities = std::list<persistence::City>{updateCity2};

    database.upsert(data);

    const persistence::Data expect{year, {}, {city1, updateCity2}};

    EXPECT_EQ(database.load(year), expect);
}

TEST_F(DatabaseTest, RemoveAllCities)
{
    int year = 1900;
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    const persistence::Data data{year, {}, {city1, city2}};
    const persistence::Data remove{year, {}, {city1, city2}};
    const persistence::Data expect{year};

    database.upsert(data);
    database.remove(remove);

    EXPECT_EQ(database.load(year), expect);
}

TEST_F(DatabaseTest, RemoveOneCity)
{
    int year = 1900;
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    const persistence::Data data{year, {}, {city1, city2}};
    const persistence::Data remove{year, {}, {city2}};
    const persistence::Data expect{year, {}, {city1}};

    database.upsert(data);
    database.remove(remove);

    EXPECT_EQ(database.load(year), expect);
}

TEST_F(DatabaseTest, InsertEvent)
{
    int year = 1900;
    const persistence::Note note{"Test"};
    const persistence::Data data{year, {}, {}, note};

    database.upsert(data);

    EXPECT_EQ(database.load(year), data);
}

TEST_F(DatabaseTest, UpdateEvent)
{
    int year = 1900;
    persistence::Note note{"Test"};
    persistence::Data data{year, {}, {}, note};

    database.upsert(data);

    note.text = "Update";
    data.note = note;

    database.upsert(data);

    EXPECT_EQ(database.load(year), data);
}

TEST_F(DatabaseTest, UpdateEventExistsInTwoYears)
{
    persistence::Note note{"Test"};
    persistence::Data data1{1900, {}, {}, note};
    persistence::Data data2{1901, {}, {}, note};

    database.upsert(data1);
    database.upsert(data2);

    data1.note->text = "Update";

    database.upsert(data1);

    EXPECT_EQ(database.load(1900), data1);
    EXPECT_EQ(database.load(1901), data2);
}

TEST_F(DatabaseTest, RemoveEvent)
{
    int year = 1900;
    const persistence::Note note{"Test"};
    const persistence::Data data{year, {}, {}, note};
    const persistence::Data remove{year, {}, {}, note};

    database.upsert(data);
    database.remove(remove);

    EXPECT_EQ(database.load(year), persistence::Data{year});
}

TEST_F(DatabaseTest, RemoveEventExistsInTwoYears)
{
    persistence::Note note{"Test"};
    persistence::Data data1{1900, {}, {}, note};
    persistence::Data data2{1901, {}, {}, note};
    persistence::Data expect{1900};

    database.upsert(data1);
    database.upsert(data2);

    database.remove(data1);

    EXPECT_EQ(database.load(1900), expect);
    EXPECT_EQ(database.load(1901), data2);
}

TEST_F(DatabaseTest, InsertAllTogether)
{
    int year = 1900;
    const persistence::Note note{"Test"};
    const persistence::Country country1{"Country1", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{"Country2", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::City city1{"One", {1,2}};
    const persistence::City city2{"Two", {3,4}};
    persistence::Data data{year, {country1, country2}, {city1, city2}, note};

    database.upsert(data);

    EXPECT_EQ(database.load(year), data);
}

TEST_F(DatabaseTest, LoadCountryList)
{
    int year = 1900;
    std::string name1 = "one";
    std::string name2 = "two";
    const persistence::Country country1{name1, {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{name2, {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    persistence::Data data{year, {country1, country2}};
    database.upsert(data);
    std::vector<std::string> expected{name1, name2};

    EXPECT_EQ(database.loadCountryList(year), expected);
}

TEST_F(DatabaseTest, LoadCityList)
{
    int year = 1900;
    std::string name1 = "One";
    std::string name2 = "Two";
    const persistence::City city1{name1, {1,2}};
    const persistence::City city2{name2, {3,4}};
    persistence::Data data{year, {}, {city1, city2}};
    database.upsert(data);
    std::vector<std::string> expected{name1, name2};

    EXPECT_EQ(database.loadCityList(year), expected);
}

TEST_F(DatabaseTest, LoadNote)
{
    int year = 1900;
    const persistence::Note note{"Test"};
    persistence::Data data{year, {}, {}, note};
    database.upsert(data);

    EXPECT_EQ(database.loadNote(year), note);
}

TEST_F(DatabaseTest, RemoveNoteWithTestNotMatch)
{
    int year = 1900;
    const persistence::Note note{"Test"};
    const persistence::Country country1{"Country1", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{"Country2", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::City city1{"One", {1,2}};
    const persistence::City city2{"Two", {3,4}};
    persistence::Data data{year, {country1, country2}, {city1, city2}, note};
    database.upsert(data);
    
    persistence::Data remove{year, {}, {}, persistence::Note{""}};
    database.remove(remove);
    
    EXPECT_EQ(database.load(year), data);
}

TEST_F(DatabaseTest, RemoveNote)
{
    int year = 1900;
    const persistence::Note note{"Test"};
    const persistence::Country country1{"Country1", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{"Country2", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::City city1{"One", {1,2}};
    const persistence::City city2{"Two", {3,4}};
    persistence::Data data{year, {country1, country2}, {city1, city2}, note};
    database.upsert(data);
    
    persistence::Data remove{year, {}, {}, note};
    database.remove(remove);

    persistence::Data expected{year, {country1, country2}, {city1, city2}};
    
    EXPECT_EQ(database.load(year), expected);
}

TEST_F(DatabaseTest, NoteIsNotAffectedWhenModifyOthers)
{
    int year = 1900;
    const persistence::Data data1{year, {}, {}, persistence::Note{"Test"}};
    const persistence::Country country{"name", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data data2{year, std::list<persistence::Country>{country}};
    database.upsert(data1);
    database.upsert(data2);
    const persistence::Data expected{year, std::list<persistence::Country>{country}, {}, persistence::Note{"Test"}};

    EXPECT_EQ(database.load(year), expected);
}

TEST_F(DatabaseTest, LoadCountry)
{
    int year = 1900;
    std::string name1 = "one";
    std::string name2 = "two";
    const persistence::Country country1{name1, {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{name2, {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    persistence::Data data{year, {country1, country2}};
    database.upsert(data);

    const auto ret = database.loadCountry(year, name2);

    EXPECT_TRUE(ret);

    EXPECT_EQ(*ret, country2);
}

TEST_F(DatabaseTest, LoadCityWithYear)
{
    int year = 1900;
    std::string name1 = "One";
    std::string name2 = "Two";
    const persistence::City city1{name1, {1,2}};
    const persistence::City city2{name2, {3,4}};
    persistence::Data data{year, {}, {city1, city2}};
    database.upsert(data);

    const auto ret = database.loadCity(year, name2);

    EXPECT_TRUE(ret);

    EXPECT_EQ(*ret, city2);
}

TEST_F(DatabaseTest, LoadCityWithoutYear)
{
    int year = 1900;
    std::string name1 = "One";
    std::string name2 = "Two";
    const persistence::City city1{name1, {1,2}};
    const persistence::City city2{name2, {3,4}};
    persistence::Data data{year, {}, {city1, city2}};
    database.upsert(data);

    const auto ret = database.loadCity(name2);

    EXPECT_TRUE(ret);

    EXPECT_EQ(*ret, city2);

    EXPECT_FALSE(database.loadCity("Three"));
}

TEST_F(DatabaseTest, LoadAllCities)
{
    std::vector<std::string> names{"one", "two"};
    const persistence::City city1{names[0], {1,2}};
    const persistence::City city2{names[1], {3,4}};
    database.upsert(persistence::Data{1900, {}, {city1}});
    database.upsert(persistence::Data{2000, {}, {city2}});

    EXPECT_EQ(database.loadCityList(), names);
}
}