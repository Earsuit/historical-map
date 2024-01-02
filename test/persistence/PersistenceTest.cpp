#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"

#include "src/persistence/Persistence.h"

#include <gtest/gtest.h>
#include <iostream>
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

TEST_F(PersistenceTest, Simple)
{
    
}


}