#include "src/model/DatabaseModel.h"

namespace model {
constexpr int MIN_YEAR = -3000;
constexpr int MAX_YEAR = 1911;
constexpr int INVALID_YEAR = 0;

DatabaseModel::DatabaseModel():
    logger{spdlog::get(logger::LOGGER_NAME)},
    database{std::make_shared<sqlpp::sqlite3::connection_config>(DATABASE_NAME, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE)},
    currentYear{QIN_DYNASTY}
{
}

DatabaseModel& DatabaseModel::getInstance()
{
    static DatabaseModel model;
    return model;
}

int DatabaseModel::getMaxYear() const noexcept
{
    return MAX_YEAR;
}

int DatabaseModel::getMinYear() const noexcept
{
    return MIN_YEAR;
}

bool DatabaseModel::setYear(int newYear) noexcept
{
    if (newYear < MIN_YEAR || newYear > MAX_YEAR) {
        return false;
    }

    if (newYear == INVALID_YEAR) {
        newYear = 1;
    }

    currentYear = newYear;

    return true;
}

bool DatabaseModel::moveYearForward() noexcept
{
    if (currentYear + 1 > MAX_YEAR) {
        return false;
    }

    currentYear++;

    if (currentYear == INVALID_YEAR) {
        currentYear++;
    }

    return true;
}

bool DatabaseModel::moveYearBackward() noexcept
{
    if (currentYear - 1 < MIN_YEAR) {
        return false;
    }

    currentYear--;

    if (currentYear == INVALID_YEAR) {
        currentYear--;
    }

    return true;
}

persistence::Data DatabaseModel::loadHistoricalInfo()
{
    return loadHistoricalInfo(currentYear);
}

persistence::Data DatabaseModel::loadHistoricalInfo(int year)
{
    logger->debug("Load item from database for year {}.", year);
    std::scoped_lock lk{lock};
    return database.load(year);
}

void DatabaseModel::updateHistoricalInfo(const persistence::Data& info)
{
    logger->debug("Update item to database for year {}.", info.year);
    std::scoped_lock lk{lock};
    database.upsert(info);
}

void DatabaseModel::removeHistoricalInfo(const persistence::Data& info)
{
    logger->debug("Remove item from database for year {}.", info.year);
    std::scoped_lock lk{lock};
    database.remove(info);
}
}