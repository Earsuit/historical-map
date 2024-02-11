#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"

#include "src/persistence/Persistence.h"

#include <gtest/gtest.h>
#include <cstdio>

namespace {
using namespace sqlpp::sqlite3;

// https://www.sqlite.org/inmemorydb.html
// share the same in memory database from two connections
constexpr auto DATABASE_NAME = "file:memdb1?mode=memory&cache=shared";

class PersistenceTest : public ::testing::Test {
public:
    PersistenceTest():
        persistence{config},
        monitor{config}
    {
    }

    ~PersistenceTest()
    {
        std::remove(DATABASE_NAME);
    }

    std::shared_ptr<connection_config> config = std::make_shared<connection_config>(DATABASE_NAME, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,  "", true);
    persistence::Persistence<connection, connection_config> persistence;
    connection monitor;
};

TEST_F(PersistenceTest, InsertEmpty)
{
    const persistence::Data data{1900};

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(1900), data);
}

TEST_F(PersistenceTest, RemoveEmpty)
{
    const persistence::Data data{1900};

    persistence.remove(data);

    EXPECT_EQ(persistence.load(1900), data);
}

TEST_F(PersistenceTest, LoadNonExistYear)
{
    EXPECT_EQ(persistence.load(2000), persistence::Data{2000});
}

TEST_F(PersistenceTest, InsertOneCountry)
{
    const persistence::Country country{"TestCountry", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Data data{1900, {country}};

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(1900), data);
}

TEST_F(PersistenceTest, InsertDuplicateCountries)
{
    const persistence::Country country{"TestCountry", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Data data{1900, {country}};

    persistence.upsert(data);
    persistence.upsert(data);

    EXPECT_EQ(persistence.load(1900), data);
    
    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(PersistenceTest, InsertTwoCountriesDifferentYear)
{
    int year1 = 1900;
    const persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Data data1{year1, {country1}};

    int year2 = 2000;
    const persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::Data data2{year2, {country2}};

    persistence.upsert(data1);
    persistence.upsert(data2);

    EXPECT_EQ(persistence.load(year1), data1);
    EXPECT_EQ(persistence.load(year2), data2);
}

TEST_F(PersistenceTest, InsertTwoCountriesSameYearSeparately)
{
    int year = 1900;
    const persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Data data1{year, {country1}};

    const persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::Data data2{year, {country2}};

    persistence.upsert(data1);
    persistence.upsert(data2);
    
    const persistence::Data expect{year, {country1, country2}};
    EXPECT_EQ(persistence.load(year), expect);
}

TEST_F(PersistenceTest, InsertTwoCountriesSameYearTogether)
{
    int year = 1900;
    const persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::Data data{year, {country1, country2}};

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

TEST_F(PersistenceTest, UpdateOneOfTheCountry)
{
    int year = 1900;
    const persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::Country updateCountry2{"Two", {persistence::Coordinate{9,10}, persistence::Coordinate{11,12}}};
    persistence::Data data{year, {country1, country2}};

    persistence.upsert(data);

    data.countries = std::list<persistence::Country>{updateCountry2};

    persistence.upsert(data);

    const persistence::Data expect{year, {country1, updateCountry2}};

    EXPECT_EQ(persistence.load(year), expect);

    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 2);
}

TEST_F(PersistenceTest, InsertTwoCountriesWithSameContourAtDifferentYear)
{
    int year1 = 1900;
    int year2 = 2000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data data1{year1, {country1}};
    persistence::Data data2{year2, {country2}};

    persistence.upsert(data1);
    persistence.upsert(data2);

    EXPECT_EQ(persistence.load(year1), data1);
    EXPECT_EQ(persistence.load(year2), data2);

    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(PersistenceTest, RemoveOneCountryDifferentBorder)
{
    int year = 1900;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    persistence::Data data{year, {country1, country2}};
    persistence::Data remove{year, {country2}};
    persistence::Data expect{year, {country1}};

    persistence.upsert(data);

    persistence.remove(remove);

    EXPECT_EQ(persistence.load(year), expect);

    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 1);
}

TEST_F(PersistenceTest, RemoveOneCountrySameBorderSameName)
{
    int year1 = 1900;
    int year2 = 2000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data remove{year2, {country2}};
    persistence::Data expect{year1, {country1}};

    persistence.upsert(persistence::Data{year1, {country1}});
    persistence.upsert(persistence::Data{year2, {country2}});

    persistence.remove(remove);

    EXPECT_EQ(persistence.load(year1), expect);
}

TEST_F(PersistenceTest, RemoveOneCountrySameBorderDifferentName)
{
    int year1 = 1900;
    int year2 = 2000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data remove{year2, {country2}};
    persistence::Data expect{year1, {country1}};

    persistence.upsert(persistence::Data{year1, {country1}});
    persistence.upsert(persistence::Data{year2, {country2}});

    persistence.remove(remove);

    EXPECT_EQ(persistence.load(year1), expect);
}

TEST_F(PersistenceTest, RemoveOneCountryFromWrongYear)
{
    int year1 = 1900;
    int year2 = 2000;
    int year3 = 3000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data remove{year3, {country2}};
    persistence::Data expect1{year1, {country1}};
    persistence::Data expect2{year2, {country2}};

    persistence.upsert(persistence::Data{year1, {country1}});
    persistence.upsert(persistence::Data{year2, {country2}});

    persistence.remove(remove);

    EXPECT_EQ(persistence.load(year1), expect1);
    EXPECT_EQ(persistence.load(year2), expect2);
}

TEST_F(PersistenceTest, RemoveAllCountries)
{
    int year = 1900;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data remove{year, {country1, country2}};
    persistence::Data expect{year};

    persistence.upsert(persistence::Data{year, {country1, country2}});

    persistence.remove(remove);

    EXPECT_EQ(persistence.load(year), expect);

    int count = 0;
    for (const auto& row : monitor(sqlpp::select(all_of(persistence::BORDERS)).from(persistence::BORDERS).unconditionally())) {
        count++;
    }
    EXPECT_EQ(count, 0);
}

TEST_F(PersistenceTest, UpdateBorderUsedByMultipleYears)
{
    int year1 = 1900;
    int year2 = 1901;
    persistence::Country country{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country update{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}, persistence::Coordinate{5,6}}};
    const persistence::Data expectedYear1{year1, {update}};
    const persistence::Data expectedYear2{year2, {country}};

    persistence.upsert(persistence::Data{year1, {country}});
    persistence.upsert(persistence::Data{year2, {country}});

    persistence.upsert(persistence::Data{year1, {update}});

    EXPECT_EQ(persistence.load(year1), expectedYear1);
    EXPECT_EQ(persistence.load(year2), expectedYear2);
}

TEST_F(PersistenceTest, UpdateBorderForMultipleYearsUsedByMultipleYears)
{
    persistence::Country country{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country update{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}, persistence::Coordinate{5,6}}};
    const persistence::Data expectedYear1{1900, {update}};
    const persistence::Data expectedYear2{1901, {update}};
    const persistence::Data expectedYear3{1902, {country}};
    const persistence::Data expectedYear4{1903, {country}};

    persistence.upsert(persistence::Data{1900, {country}});
    persistence.upsert(persistence::Data{1901, {country}});
    persistence.upsert(persistence::Data{1902, {country}});
    persistence.upsert(persistence::Data{1903, {country}});

    persistence.upsert(persistence::Data{1900, {update}});
    persistence.upsert(persistence::Data{1901, {update}});

    EXPECT_EQ(persistence.load(1900), expectedYear1);
    EXPECT_EQ(persistence.load(1901), expectedYear2);
    EXPECT_EQ(persistence.load(1902), expectedYear3);
    EXPECT_EQ(persistence.load(1903), expectedYear4);
}

TEST_F(PersistenceTest, UpdateBorderMultipleTimes)
{
    persistence::Country country{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country update{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}, persistence::Coordinate{5,6}}};
    const persistence::Data expectedYear1{1900, {country}};
    const persistence::Data expectedYear2{1901, {country}};
    const persistence::Data expectedYear3{1902, {country}};
    const persistence::Data expectedYear4{1903, {country}};

    persistence.upsert(persistence::Data{1900, {country}});
    persistence.upsert(persistence::Data{1901, {country}});
    persistence.upsert(persistence::Data{1902, {country}});
    persistence.upsert(persistence::Data{1903, {country}});

    persistence.upsert(persistence::Data{1900, {update}});
    persistence.upsert(persistence::Data{1901, {update}});

    persistence.upsert(persistence::Data{1900, {country}});
    persistence.upsert(persistence::Data{1901, {country}});

    EXPECT_EQ(persistence.load(1900), expectedYear1);
    EXPECT_EQ(persistence.load(1901), expectedYear2);
    EXPECT_EQ(persistence.load(1902), expectedYear3);
    EXPECT_EQ(persistence.load(1903), expectedYear4);
}

TEST_F(PersistenceTest, UpdateBorderUsedByMultipleCountries)
{
    int year = 1900;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country update{"Two", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}, persistence::Coordinate{5,6}}};
    const persistence::Data expectedYear{year, {country1, update}};

    persistence.upsert(persistence::Data{year, {country1, country2}});
    persistence.upsert(persistence::Data{year, {update}});

    EXPECT_EQ(persistence.load(year), expectedYear);
}

TEST_F(PersistenceTest, InsertOneCity)
{
    int year = 1900;
    persistence::Data data{year};
    data.cities.emplace_back("One", persistence::Coordinate{1, 2});

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

TEST_F(PersistenceTest, InsertDuplicateCities)
{
    int year = 1900;
    persistence::Data data{year};
    data.cities.emplace_back("One", persistence::Coordinate{1, 2});

    persistence.upsert(data);
    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

TEST_F(PersistenceTest, InsertOneCountryWithDifferentContourAtDifferentYear)
{
    int year1 = 1900;
    int year2 = 2000;
    persistence::Country country1{"One", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Country country2{"One", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    persistence::Data data1{year1, {country1}};
    persistence::Data data2{year2, {country2}};

    persistence.upsert(data1);
    persistence.upsert(data2);

    EXPECT_EQ(persistence.load(year1), data1);
    EXPECT_EQ(persistence.load(year2), data2);
}

TEST_F(PersistenceTest, InsertTwoCitiesSameYearSeparately)
{
    int year = 1900;
    persistence::Data data1{year}, data2{year};
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    data1.cities.emplace_back(city1);
    data2.cities.emplace_back(city2);

    persistence.upsert(data1);
    persistence.upsert(data2);

    const persistence::Data expect{year, {}, {city1, city2}};
    EXPECT_EQ(persistence.load(year), expect);
}

TEST_F(PersistenceTest, InsertTwoCitiesSameYearTogether)
{
    int year = 1900;
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    const persistence::Data data{year, {}, {city1, city2}};

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

TEST_F(PersistenceTest, UpdateOneOfTheCity)
{
    int year = 1900;
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    const persistence::City updateCity2{"Two", {5,6}};
    persistence::Data data{year, {}, {city1, city2}};

    persistence.upsert(data);

    data.cities = std::list<persistence::City>{updateCity2};

    persistence.upsert(data);

    const persistence::Data expect{year, {}, {city1, updateCity2}};

    EXPECT_EQ(persistence.load(year), expect);
}

TEST_F(PersistenceTest, RemoveAllCities)
{
    int year = 1900;
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    const persistence::Data data{year, {}, {city1, city2}};
    const persistence::Data remove{year, {}, {city1, city2}};
    const persistence::Data expect{year};

    persistence.upsert(data);
    persistence.remove(remove);

    EXPECT_EQ(persistence.load(year), expect);
}

TEST_F(PersistenceTest, RemoveOneCity)
{
    int year = 1900;
    const persistence::City city1{"One", {1, 2}};
    const persistence::City city2{"Two", {3,4}};
    const persistence::Data data{year, {}, {city1, city2}};
    const persistence::Data remove{year, {}, {city2}};
    const persistence::Data expect{year, {}, {city1}};

    persistence.upsert(data);
    persistence.remove(remove);

    EXPECT_EQ(persistence.load(year), expect);
}

TEST_F(PersistenceTest, InsertEvent)
{
    int year = 1900;
    const persistence::Note note{"Test"};
    const persistence::Data data{year, {}, {}, note};

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

TEST_F(PersistenceTest, UpdateEvent)
{
    int year = 1900;
    persistence::Note note{"Test"};
    persistence::Data data{year, {}, {}, note};

    persistence.upsert(data);

    note.text = "Update";
    data.note = note;

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

TEST_F(PersistenceTest, UpdateEventExistsInTwoYears)
{
    persistence::Note note{"Test"};
    persistence::Data data1{1900, {}, {}, note};
    persistence::Data data2{1901, {}, {}, note};

    persistence.upsert(data1);
    persistence.upsert(data2);

    data1.note.text = "Update";

    persistence.upsert(data1);

    EXPECT_EQ(persistence.load(1900), data1);
    EXPECT_EQ(persistence.load(1901), data2);
}

TEST_F(PersistenceTest, RemoveEvent)
{
    int year = 1900;
    const persistence::Note note{"Test"};
    const persistence::Data data{year, {}, {}, note};
    const persistence::Data remove{year, {}, {}, note};

    persistence.upsert(data);
    persistence.remove(remove);

    EXPECT_EQ(persistence.load(year), persistence::Data{year});
}

TEST_F(PersistenceTest, RemoveEventExistsInTwoYears)
{
    persistence::Note note{"Test"};
    persistence::Data data1{1900, {}, {}, note};
    persistence::Data data2{1901, {}, {}, note};
    persistence::Data expect{1900};

    persistence.upsert(data1);
    persistence.upsert(data2);

    persistence.remove(data1);

    EXPECT_EQ(persistence.load(1900), expect);
    EXPECT_EQ(persistence.load(1901), data2);
}

TEST_F(PersistenceTest, InsertAllTogether)
{
    int year = 1900;
    const persistence::Note note{"Test"};
    const persistence::Country country1{"Country1", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{"Country2", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::City city1{"One", {1,2}};
    const persistence::City city2{"Two", {3,4}};
    persistence::Data data{year, {country1, country2}, {city1, city2}, note};

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

}