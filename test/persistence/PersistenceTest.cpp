#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"

#include "src/persistence/Persistence.h"

#include <gtest/gtest.h>

namespace {
using namespace sqlpp::sqlite3;

class PersistenceTest : public ::testing::Test {
public:
    PersistenceTest():
        persistence{std::make_shared<connection_config>("file::memory:?cache=shared", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,  "", true)}
    {
    }

    persistence::Persistence<connection_pool, connection_config> persistence;
};

TEST_F(PersistenceTest, InsertEmpty)
{
    const persistence::Data data{1900};

    persistence.upsert(data);

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

    data.countries = std::vector<persistence::Country>{updateCountry2};

    persistence.upsert(data);

    const persistence::Data expect{year, {country1, updateCountry2}};

    EXPECT_EQ(persistence.load(year), expect);
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

    data.cities = std::vector<persistence::City>{updateCity2};

    persistence.upsert(data);

    const persistence::Data expect{year, {}, {city1, updateCity2}};

    EXPECT_EQ(persistence.load(year), expect);
}

TEST_F(PersistenceTest, InsertEvent)
{
    int year = 1900;
    const persistence::Event event{"Test"};
    const persistence::Data data{year, {}, {}, event};

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

TEST_F(PersistenceTest, UpdateEvent)
{
    int year = 1900;
    persistence::Event event{"Test"};
    persistence::Data data{year, {}, {}, event};

    persistence.upsert(data);

    event.description = "Update";
    data.event = event;

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

TEST_F(PersistenceTest, InsertAllTogether)
{
    int year = 1900;
    const persistence::Event event{"Test"};
    const persistence::Country country1{"Country1", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    const persistence::Country country2{"Country2", {persistence::Coordinate{5,6}, persistence::Coordinate{7,8}}};
    const persistence::City city1{"One", {1,2}};
    const persistence::City city2{"Two", {3,4}};
    persistence::Data data{year, {country1, country2}, {city1, city2}, event};

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(year), data);
}

}