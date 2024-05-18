#include "src/presentation/HistoricalInfoPresenter.h"

namespace presentation {
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

std::vector<std::string> HistoricalInfoPresenter::handleRequestCountryList() const
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        return info->getCountryList();
    }

    return {};
}

std::list<persistence::Coordinate> HistoricalInfoPresenter::handleRequestContour(const std::string& name) const
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        return info->getCountry(name).borderContour;
    }

    return {};
}

void HistoricalInfoPresenter::handleUpdateContour(const std::string& name, int idx, const persistence::Coordinate& coordinate)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        auto& contour = info->getCountry(name).borderContour;
        auto it = std::next(contour.begin(), idx);
        it->latitude = coordinate.latitude;
        it->longitude = coordinate.longitude;
    }
}

void HistoricalInfoPresenter::handleDeleteFromContour(const std::string& name, int idx)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        auto& contour = info->getCountry(name).borderContour;
        auto it = std::next(contour.begin(), idx);
        contour.erase(it);
    }
}

std::vector<std::string> HistoricalInfoPresenter::handleRequestCityList() const
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        return info->getCityList();
    }

    return {};
}

std::optional<persistence::Coordinate> HistoricalInfoPresenter::handleRequestCityCoordinate(const std::string& name) const
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        return info->getCity(name).coordinate;
    }

    return std::nullopt;
}

void HistoricalInfoPresenter::handleUpdateCityCoordinate(const std::string& name, const persistence::Coordinate& coord)
{   
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        info->getCity(name).coordinate = coord;
    } else {
        logger->error("Update city {} coordinate fail because historical info is null from source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleClearHistoricalInfo()
{
    dynamicInfoModel.removeHistoricalInfoFromSource(source);
}
}