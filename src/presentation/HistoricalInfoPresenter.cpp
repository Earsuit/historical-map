#include "src/presentation/HistoricalInfoPresenter.h"

namespace presentation {
void HistoricalInfoPresenter::handleDisplayCountries()
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        for (const auto& name: info->getCountryList()) {
            view.displayCountry(name);
        }
    } else {
        logger->trace("Historical info is null from source {} in handleDisplayCountries", source);
    }
}

void HistoricalInfoPresenter::handleDisplayCities()
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        for (const auto& name: info->getCityList()) {
            view.displayCity(name);
        }
    } else {
        logger->trace("Historical info is null from source {} in handleDisplayCities", source);
    }
}

void HistoricalInfoPresenter::handleDisplayCity(const std::string& name)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        if (info->containsCity(name)) {
            auto& city = info->getCity(name); 
            city.coordinate = view.displayCoordinate(city.coordinate);
        }
    } else {
        logger->trace("Historical info is null from source {} in handleDisplayCity", source);
    }
}

void HistoricalInfoPresenter::handleDisplayCountry(const std::string& name)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        if (info->containsCountry(name)) {
            auto& country = info->getCountry(name);

            for (auto& coordinate : country.borderContour) {
                coordinate = view.displayCoordinate(coordinate);
            }
        }
    } else {
        logger->trace("Historical info is null from source {} in handleDisplayCountry", source);
    }
}

void HistoricalInfoPresenter::handleAddCountry(const std::string& name)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        info->addCountry(name);
    } else {
        logger->error("Add country fail because historical info is null from source {}", source);
    }
}

void HistoricalInfoPresenter::handleExtendContour(const std::string& name, 
                                                        const persistence::Coordinate& coordinate)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        if (info->containsCountry(name)) {
            auto& country = info->getCountry(name);
            country.borderContour.emplace_back(coordinate);
        }
    } else {
        logger->error("Extend contour fail because historical info is null from source {}", source);
    }
}

void HistoricalInfoPresenter::handleAddCity(const std::string& name, const persistence::Coordinate& coordinate)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        info->addCity(name, coordinate);
    } else {
        logger->error("Add city fail because historical info is null from source {}", source);
    }
}

void HistoricalInfoPresenter::handleRemoveCountry(const std::string& name)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        info->removeCountry(name);
    } else {
        logger->error("Remove country fail because historical info is null from source {}", source);
    }
}

void HistoricalInfoPresenter::handleRemoveCity(const std::string& name)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        info->removeCity(name);
    } else {
        logger->error("Remove city fail because historical info is null from source {}", source);
    }
}

std::optional<std::string> HistoricalInfoPresenter::handleGetNote() const noexcept
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        return info->getNote().text;
    } else {
        logger->trace("Get note fail because historical info is null from source {}", source);
    }

    return std::nullopt;
}

void HistoricalInfoPresenter::handleUpdateNote(const std::string& text)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        info->getNote().text = text;
    } else {
        logger->error("Update note fail because historical info is null from source {}", source);
    }
}

void HistoricalInfoPresenter::setHoveredCoord(const persistence::Coordinate& coordinate)
{
    dynamicInfoModel.setHoveredCoord(coordinate);
}

void HistoricalInfoPresenter::clearHoveredCoord()
{
    dynamicInfoModel.clearHoveredCoord();
}
}