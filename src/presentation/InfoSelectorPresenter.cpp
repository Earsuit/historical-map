#include "src/presentation/InfoSelectorPresenter.h"
#include "src/logger/LoggerManager.h"

#include <chrono>

namespace presentation {
using namespace std::chrono_literals;

constexpr auto LOGGER_NAME = "InfoSelectorPresenter";

InfoSelectorPresenter::InfoSelectorPresenter(const std::string& fromSource, const std::string& toSource):
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    cacheModel{model::CacheModel::getInstance()},
    fromSource{fromSource},
    toSource{toSource}
{
    cacheModel.addSource(toSource);

    util::signal::connect(&cacheModel, 
                          &model::CacheModel::onCountryUpdate, 
                          this, 
                          &InfoSelectorPresenter::onUpdate);
    util::signal::connect(&cacheModel,
                          &model::CacheModel::onCityUpdate,
                          this,
                          &InfoSelectorPresenter::onUpdate);
    util::signal::connect(&cacheModel,
                          &model::CacheModel::onNoteUpdate,
                          this,
                          &InfoSelectorPresenter::onUpdate);
}

InfoSelectorPresenter::~InfoSelectorPresenter()
{
    util::signal::disconnectAll(&cacheModel, 
                                &model::CacheModel::onCountryUpdate, 
                                this);
    util::signal::disconnectAll(&cacheModel,
                                &model::CacheModel::onCityUpdate,
                                this);
    util::signal::disconnectAll(&cacheModel,
                                &model::CacheModel::onNoteUpdate,
                                this);

    cacheModel.removeSource(toSource);
}

void InfoSelectorPresenter::upsertHistoricalStroageIfNotExists(int year)
{
    if (cacheModel.containsHistoricalInfo(toSource, year)) {
        return;
    }

    cacheModel.upsert(toSource, persistence::Data{year});
}

void InfoSelectorPresenter::handleSelectCountry(const std::string& name)
{
    const auto year = databaseModel.getYear();
    upsertHistoricalStroageIfNotExists(year);
    if (const auto country = cacheModel.getCountry(fromSource, year, name); country) {
        if (cacheModel.addCountry(toSource, year, country.value())) {
            return;
        }
    }

    logger.error("Select country {} fail", name);
}

void InfoSelectorPresenter::handleSelectCity(const std::string& name)
{
    const auto year = databaseModel.getYear();
    upsertHistoricalStroageIfNotExists(year);
    if (const auto city = cacheModel.getCity(fromSource, year, name); city) {
        if (cacheModel.addCity(toSource, year, city.value())) {
            return;
        }
    }

    logger.error("Select city {} fail", name);
}

void InfoSelectorPresenter::handleSelectNote()
{
    const auto year = databaseModel.getYear();
    upsertHistoricalStroageIfNotExists(year);
    if (const auto note = cacheModel.getNote(fromSource, year); note) {
        if (cacheModel.addNote(toSource, year, note.value())) {
            return;
        }
    }

    logger.error("Select note fail");
}

void InfoSelectorPresenter::handleDeselectCountry(const std::string& name)
{
    const auto year = databaseModel.getYear();
    cacheModel.removeCountry(toSource, year, name);
    cacheModel.clearRemoved(toSource, year);
}

void InfoSelectorPresenter::handleDeselectCity(const std::string& name)
{
    const auto year = databaseModel.getYear();
    cacheModel.removeCity(toSource, year, name);
    cacheModel.clearRemoved(toSource, year);
}

void InfoSelectorPresenter::handleDeselectNote()
{
    const auto year = databaseModel.getYear();
    cacheModel.removeNote(toSource, year);
    cacheModel.clearRemoved(toSource, year);
}

bool InfoSelectorPresenter::handkeCheckIsCountrySelected(const std::string& name)
{
    return cacheModel.containsCountry(toSource, databaseModel.getYear(), name);
}

bool InfoSelectorPresenter::handleCheckIsCitySelected(const std::string& name)
{
    return cacheModel.containsCity(toSource, databaseModel.getYear(), name);
}

bool InfoSelectorPresenter::handleCheckIsNoteSelected()
{
    return cacheModel.containsNote(toSource, databaseModel.getYear());
}

bool InfoSelectorPresenter::handleCheckIsAllSelected()
{
    const auto year = databaseModel.getYear();
    const auto toCountries = cacheModel.getCountryList(toSource, year);
    const auto toCities = cacheModel.getCityList(toSource, year);
    const auto fromCountries = cacheModel.getCountryList(fromSource, year);
    const auto fromCities = cacheModel.getCityList(fromSource, year);
    std::unordered_set<std::string> selectedCountries{toCountries.cbegin(), toCountries.cend()};
    std::unordered_set<std::string> selectedCitiess{toCities.cbegin(), toCities.cend()};

    if (toCountries.size() != fromCountries.size() || toCities.size() != fromCities.size()) {
        return false;
    }

    if (toCountries.empty() && 
        toCities.empty() &&
        !cacheModel.containsNote(toSource, year)) {
        return false;
    }

    for (const auto& country : fromCountries) {
        if (!selectedCountries.contains(country)) {
            return false;
        }
    }

    for (const auto& city : fromCities) {
        if (!selectedCitiess.contains(city)) {
            return false;
        }
    }

    return cacheModel.containsNote(toSource, year) == cacheModel.containsNote(fromSource, year);
}

void InfoSelectorPresenter::handleSelectAll()
{
    if (auto data = cacheModel.getData(fromSource, databaseModel.getYear()); data) {
        cacheModel.upsert(toSource, *data);
    }
}

void InfoSelectorPresenter::handleDeselectAll()
{
    cacheModel.removeHistoricalInfoFromSource(toSource, databaseModel.getYear());
}

void InfoSelectorPresenter::handleSelectAllForMultipleYears(int startYear, int endYear)
{
    total = endYear - startYear + 1;
    stopTask = false;
    task = std::async(std::launch::async, [this, startYear, endYear](){
        for (int year = startYear; year <= endYear; year++) {
            if (stopTask) {
                return;
            }

            this->databaseModel.setYear(year);
            
            if (!this->cacheModel.containsHistoricalInfo(this->fromSource, year)) {
                this->cacheModel.upsert(this->fromSource, 
                                              this->databaseModel.loadHistoricalInfo(year));
            }

            this->handleSelectAll();
            this->progress++;
        }
    });
}

float InfoSelectorPresenter::handleGetSelectAllForMultipleYearsProgress() const noexcept
{
    return static_cast<float>(progress) / total;
}

bool InfoSelectorPresenter::handleCheckSelectAllForMultipleYearsComplete()
{
    if (task.valid() && task.wait_for(0s) == std::future_status::ready) {
        task.get();
        return true;
    } else {
        return false;
    }
}

void InfoSelectorPresenter::onUpdate(const std::string& source, int year)
{
    if (source == this->toSource && year == databaseModel.getYear()) {
        logger.debug("InfoSelectorPresenter onUpdate for source {} at year {}", source, year);
        setRefreshSelectAll();
    }
}
}