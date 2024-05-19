#include "src/presentation/InfoSelectorPresenter.h"

#include "src/logger/Util.h"

namespace presentation {
InfoSelectorPresenter::InfoSelectorPresenter(const std::string& fromSource, const std::string& toSource):
    logger{spdlog::get(logger::LOGGER_NAME)},
    dynamicModel{model::DynamicInfoModel::getInstance()},
    fromSource{fromSource},
    toSource{toSource}
{
    dynamicModel.addSource(toSource);
}

InfoSelectorPresenter::~InfoSelectorPresenter()
{
    dynamicModel.removeSource(toSource);
}

std::shared_ptr<persistence::HistoricalStorage> InfoSelectorPresenter::upsertHistoricalStroageIfNotExists()
{
    if (auto toInfo = dynamicModel.getHistoricalInfo(toSource); toInfo) {
        return toInfo;
    }

    logger->debug("Upsert empty HistoricalStroage for target {}", toSource);
    dynamicModel.upsert(toSource, persistence::Data{dynamicModel.getCurrentYear()});
    return dynamicModel.getHistoricalInfo(toSource);
}

void InfoSelectorPresenter::handleSelectCountry(const std::string& name)
{
    if (auto fromInfo = dynamicModel.getHistoricalInfo(fromSource); fromInfo) {
        if (fromInfo->containsCountry(name)) {
            auto& country = fromInfo->getCountry(name);
            auto toInfo = upsertHistoricalStroageIfNotExists();
            toInfo->addCountry(country);
        } else {
            logger->error("Select country {} fail because it doesn't exists in {}", name, fromSource);
        }
    } else {
        logger->error("Select country {} fail due to invalid source {}", name, fromSource);
    }
}

void InfoSelectorPresenter::handleSelectCity(const std::string& name)
{
    if (auto fromInfo = dynamicModel.getHistoricalInfo(fromSource); fromInfo) {
        if (fromInfo->containsCity(name)) {
            auto& city = fromInfo->getCity(name);
            auto toInfo = upsertHistoricalStroageIfNotExists();
            toInfo->addCity(name, city.coordinate);
        } else {
            logger->error("Select city {} fail because it doesn't exists in {}", name, fromSource);
        }
    } else {
        logger->error("Select city {} fail due to invalid source {}", name, fromSource);
    }
}

void InfoSelectorPresenter::handleSelectNote()
{
    if (auto fromInfo = dynamicModel.getHistoricalInfo(fromSource); fromInfo) {
        auto toInfo = upsertHistoricalStroageIfNotExists();
        toInfo->getNote() = fromInfo->getNote();
    } else {
        logger->error("Select note fail due to invalid source {}", fromSource);
    }
}

void InfoSelectorPresenter::handleDeselectCountry(const std::string& name)
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        info->removeCountry(name);
    }
}

void InfoSelectorPresenter::handleDeselectCity(const std::string& name)
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        info->removeCity(name);
    }
}

void InfoSelectorPresenter::handleDeselectNote()
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        info->getNote().text.clear();
    }
}

bool InfoSelectorPresenter::handkeCheckIsCountrySelected(const std::string& name)
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        return info->containsCountry(name);
    }
    
    return false;
}

bool InfoSelectorPresenter::handleCheckIsCitySelected(const std::string& name)
{
    if (auto info = dynamicModel.getHistoricalInfo(toSource); info) {
        return info->containsCity(name);
    }

    return false;
}

bool InfoSelectorPresenter::handleCheckIsNoteSelected()
{
    if (auto toInfo = dynamicModel.getHistoricalInfo(toSource); toInfo) {
        if (auto fromInfo = dynamicModel.getHistoricalInfo(fromSource); fromInfo) {
            return toInfo->getNote() == fromInfo->getNote();
        } else {
            logger->error("Check note selection fail due to invalid source {}", fromSource);
            return false;
        }
    } else {
        return false;
    }
}

bool InfoSelectorPresenter::handleCheckIsAllSelected()
{
    if (auto toInfo = dynamicModel.getHistoricalInfo(toSource); toInfo) {
        if (auto fromInfo = dynamicModel.getHistoricalInfo(fromSource); fromInfo) {
            const auto countries = toInfo->getCountryList();
            const auto cities = toInfo->getCityList();
            std::unordered_set<std::string> selectedCountries{countries.cbegin(), countries.cend()};  
            std::unordered_set<std::string> selectedCitiess{cities.cbegin(), cities.cend()};  

            for (const auto& country : fromInfo->getCountryList()) {
                if (!selectedCountries.contains(country)) {
                    return false;
                }
            }

            for (const auto& city : fromInfo->getCityList()) {
                if (!selectedCitiess.contains(city)) {
                    return false;
                }
            }

            if (toInfo->getNote() != fromInfo->getNote()) {
                return false;
            }

            return true;
        } else {
            logger->error("Check select all fail due to invalid source {}", fromSource);
            return false;
        }
    } else {
        return false;
    }
}

void InfoSelectorPresenter::handleSelectAll()
{
    if (auto fromInfo = dynamicModel.getHistoricalInfo(fromSource); fromInfo) {
        dynamicModel.upsert(toSource, fromInfo->getData());
    }
}

void InfoSelectorPresenter::handleDeselectAll()
{
    dynamicModel.removeHistoricalInfoFromSource(toSource);
}
}