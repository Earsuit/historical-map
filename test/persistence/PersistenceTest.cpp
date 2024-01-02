#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"

#include "src/persistence/Persistence.h"

#include <gtest/gtest.h>

namespace {
using namespace sqlpp::sqlite3;

class PersistenceTest : public ::testing::Test {
public:
    PersistenceTest():
        persistence{std::make_shared<connection_config>(":memory:", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)}
    {
    }

    persistence::Persistence<connection_pool, connection_config> persistence;
};

TEST_F(PersistenceTest, InsertOneCountry)
{
    persistence::Country country{"TestCountry", {persistence::Coordinate{1,2}, persistence::Coordinate{3,4}}};
    persistence::Data data{1900, {country}};

    persistence.upsert(data);

    EXPECT_EQ(persistence.load(1900), data);
}


}