#include "src/presentation/InfoSelectorPresenter.h"

#include "src/logger/Util.h"

#include <chrono>

namespace presentation {
using namespace std::chrono_literals;

InfoSelectorPresenter::InfoSelectorPresenter(const std::string& fromSource, const std::string& toSource):
    logger{spdlog::get(logger::LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    dynamicInfoModel{model::DynamicInfoModel::getInstance()},
    fromSource{fromSource},
    toSource{toSource}
{
    dynamicInfoModel.addSource(toSource);

    util::signal::connect(&dynamicInfoModel, 
                          &model::DynamicInfoModel::onCountryUpdate, 
                          this, 
                          &InfoSelectorPresenter::onUpdate);
    util::signal::connect(&dynamicInfoModel,
                          &model::DynamicInfoModel::onCityUpdate,
                          this,
                          &InfoSelectorPresenter::onUpdate);
    util::signal::connect(&dynamicInfoModel,
                          &model::DynamicInfoModel::onNoteUpdate,
                          this,
                          &InfoSelectorPresenter::onUpdate);
}

InfoSelectorPresenter::~InfoSelectorPresenter()
{
    util::signal::disconnectAll(&dynamicInfoModel, 
                                &model::DynamicInfoModel::onCountryUpdate, 
                                this);
    util::signal::disconnectAll(&dynamicInfoModel,
                                &model::DynamicInfoModel::onCityUpdate,
                                this);
    util::signal::disconnectAll(&dynamicInfoModel,
                                &model::DynamicInfoModel::onNoteUpdate,
                                this);

    dynamicInfoModel.removeSource(toSource);
}

void InfoSelectorPresenter::upsertHistoricalStroageIfNotExists(int year)
{
    if (dynamicInfoModel.containsHistoricalInfo(toSource, year)) {
        return;
    }

    dynamicInfoModel.upsert(toSource, persistence::Data{year});
}

void InfoSelectorPresenter::handleSelectCountry(const std::string& name)
{
    const auto year = databaseModel.getYear();
    upsertHistoricalStroageIfNotExists(year);
    if (const auto country = dynamicInfoModel.getCountry(fromSource, year, name); country) {
        if (dynamicInfoModel.addCountry(toSource, year, country.value())) {
            return;
        }
    }

    logger->error("Select country {} fail", name);
}

void InfoSelectorPresenter::handleSelectCity(const std::string& name)
{
    const auto year = databaseModel.getYear();
    upsertHistoricalStroageIfNotExists(year);
    if (const auto city = dynamicInfoModel.getCity(fromSource, year, name); city) {
        if (dynamicInfoModel.addCity(toSource, year, city.value())) {
            return;
        }
    }

    logger->error("Select city {} fail", name);
}

void InfoSelectorPresenter::handleSelectNote()
{
    const auto year = databaseModel.getYear();
    upsertHistoricalStroageIfNotExists(year);
    if (const auto note = dynamicInfoModel.getNote(fromSource, year); note) {
        if (dynamicInfoModel.addNote(toSource, year, note.value())) {
            return;
        }
    }

    logger->error("Select note fail");
}

void InfoSelectorPresenter::handleDeselectCountry(const std::string& name)
{
    const auto year = databaseModel.getYear();
    dynamicInfoModel.removeCountry(toSource, year, name);
    dynamicInfoModel.clearRemoved(toSource, year);
}

void InfoSelectorPresenter::handleDeselectCity(const std::string& name)
{
    const auto year = databaseModel.getYear();
    dynamicInfoModel.removeCity(toSource, year, name);
    dynamicInfoModel.clearRemoved(toSource, year);
}

void InfoSelectorPresenter::handleDeselectNote()
{
    const auto year = databaseModel.getYear();
    dynamicInfoModel.removeNote(toSource, year);
    dynamicInfoModel.clearRemoved(toSource, year);
}

bool InfoSelectorPresenter::handkeCheckIsCountrySelected(const std::string& name)
{
    return dynamicInfoModel.containsCountry(toSource, databaseModel.getYear(), name);
}

bool InfoSelectorPresenter::handleCheckIsCitySelected(const std::string& name)
{
    return dynamicInfoModel.containsCity(toSource, databaseModel.getYear(), name);
}

bool InfoSelectorPresenter::handleCheckIsNoteSelected()
{
    return dynamicInfoModel.containsNote(toSource, databaseModel.getYear());
}

bool InfoSelectorPresenter::handleCheckIsAllSelected()
{
    const auto year = databaseModel.getYear();
    const auto toCountries = dynamicInfoModel.getCountryList(toSource, year);
    const auto toCities = dynamicInfoModel.getCityList(toSource, year);
    const auto fromCountries = dynamicInfoModel.getCountryList(fromSource, year);
    const auto fromCities = dynamicInfoModel.getCityList(fromSource, year);
    std::unordered_set<std::string> selectedCountries{toCountries.cbegin(), toCountries.cend()};
    std::unordered_set<std::string> selectedCitiess{toCities.cbegin(), toCities.cend()};

    if (toCountries.size() != fromCountries.size() || toCities.size() != fromCities.size()) {
        return false;
    }

    if (toCountries.empty() && 
        toCities.empty() &&
        !dynamicInfoModel.containsNote(toSource, year)) {
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

    return dynamicInfoModel.containsNote(toSource, year) == dynamicInfoModel.containsNote(fromSource, year);
}

void InfoSelectorPresenter::handleSelectAll()
{
    if (auto data = dynamicInfoModel.getData(fromSource, databaseModel.getYear()); data) {
        dynamicInfoModel.upsert(toSource, *data);
    }
}

void InfoSelectorPresenter::handleDeselectAll()
{
    dynamicInfoModel.removeHistoricalInfoFromSource(toSource, databaseModel.getYear());
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
            
            if (!this->dynamicInfoModel.containsHistoricalInfo(this->fromSource, year)) {
                this->dynamicInfoModel.upsert(this->fromSource, 
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
        logger->debug("InfoSelectorPresenter onUpdate for source {} at year {}", source, year);
        setRefreshSelectAll();
    }
}
}