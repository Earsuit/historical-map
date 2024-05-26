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
}

InfoSelectorPresenter::~InfoSelectorPresenter()
{
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
    const auto year = dynamicInfoModel.getCurrentYear();
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
    const auto year = dynamicInfoModel.getCurrentYear();
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
    const auto year = dynamicInfoModel.getCurrentYear();
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
    const auto year = dynamicInfoModel.getCurrentYear();
    dynamicInfoModel.removeCountry(toSource, year, name);
    dynamicInfoModel.clearRemoved(toSource, year);
}

void InfoSelectorPresenter::handleDeselectCity(const std::string& name)
{
    const auto year = dynamicInfoModel.getCurrentYear();
    dynamicInfoModel.removeCity(toSource, year, name);
    dynamicInfoModel.clearRemoved(toSource, year);
}

void InfoSelectorPresenter::handleDeselectNote()
{
    const auto year = dynamicInfoModel.getCurrentYear();
    dynamicInfoModel.removeNote(toSource, year);
    dynamicInfoModel.clearRemoved(toSource, year);
}

bool InfoSelectorPresenter::handkeCheckIsCountrySelected(const std::string& name)
{
    return dynamicInfoModel.containsCountry(toSource, dynamicInfoModel.getCurrentYear(), name);
}

bool InfoSelectorPresenter::handleCheckIsCitySelected(const std::string& name)
{
    return dynamicInfoModel.containsCity(toSource, dynamicInfoModel.getCurrentYear(), name);
}

bool InfoSelectorPresenter::handleCheckIsNoteSelected()
{
    return dynamicInfoModel.containsNote(toSource, dynamicInfoModel.getCurrentYear());
}

bool InfoSelectorPresenter::handleCheckIsAllSelected()
{
    const auto year = dynamicInfoModel.getCurrentYear();
    const auto toCountries = dynamicInfoModel.getCountryList(toSource, year);
    const auto toCities = dynamicInfoModel.getCityList(toSource, year);
    const auto fromCountries = dynamicInfoModel.getCountryList(fromSource, year);
    const auto fromCities = dynamicInfoModel.getCityList(fromSource, year);
    std::unordered_set<std::string> selectedCountries{toCountries.cbegin(), toCountries.cend()};
    std::unordered_set<std::string> selectedCitiess{toCities.cbegin(), toCities.cend()};

    if (toCountries.size() != fromCountries.size() || toCities.size() != fromCountries.size()) {
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
    if (auto data = dynamicInfoModel.getData(fromSource, dynamicInfoModel.getCurrentYear()); data) {
        dynamicInfoModel.upsert(toSource, *data);
    }
}

void InfoSelectorPresenter::handleDeselectAll()
{
    dynamicInfoModel.removeHistoricalInfoFromSource(toSource);
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
            this->dynamicInfoModel.setCurrentYear(year);
            
            if (!this->dynamicInfoModel.containsHistoricalInfo(this->fromSource, year)) {
                this->dynamicInfoModel.upsert(this->fromSource, 
                                              this->databaseModel.loadHistoricalInfo());
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
}