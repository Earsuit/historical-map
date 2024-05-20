#ifndef SRC_MODEL_DATABASE_MODEL_H
#define SRC_MODEL_DATABASE_MODEL_H

#include "src/persistence/Data.h"
#include "src/persistence/Database.h"
#include "src/logger/Util.h"

#include "sqlpp11/sqlite3/sqlite3.h"
#include "sqlpp11/sqlite3/connection_config.h"
#include "spdlog/spdlog.h"

#include <memory>
#include <mutex>

namespace model {
class DatabaseModel {
public:
    static DatabaseModel& getInstance();

    bool setYear(int year) noexcept;
    bool moveYearForward() noexcept;
    bool moveYearBackward() noexcept;
    int getYear() const noexcept { return currentYear; }
    int getMaxYear() const noexcept;
    int getMinYear() const noexcept;

    persistence::Data loadHistoricalInfo();
    persistence::Data loadHistoricalInfo(int year);
    void updateHistoricalInfo(const persistence::Data& info);
    void removeHistoricalInfo(const persistence::Data& info);

    DatabaseModel(DatabaseModel&&) = delete;
    DatabaseModel(const DatabaseModel&) = delete;
    DatabaseModel& operator=(const DatabaseModel&) = delete;

private:
    constexpr static int QIN_DYNASTY = -221;
    constexpr static auto DATABASE_NAME = "HistoricalMapDB";

    std::shared_ptr<spdlog::logger> logger;
    persistence::Database<sqlpp::sqlite3::connection, sqlpp::sqlite3::connection_config> database;
    std::mutex lock;
    int currentYear;

    DatabaseModel();
};
}

#endif
