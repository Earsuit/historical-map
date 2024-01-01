#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"

#include "src/persistence/Persistence.h"

#include <gtest/gtest.h>

namespace {
using namespace sqlpp::sqlite3;

class PersistenceTest : public ::testing::Test {
public:

    persistence::Persistence<connection_pool, connection_config> persistence;
};
}