#include "src/presentation/HistoricalInfoPresenter.h"

namespace presentation {
void HistoricalInfoPresenter::handleAddCountry(const std::string& name)
{
    if (!dynamicInfoModel.addCountry(source, dynamicInfoModel.getCurrentYear(),name)) {
        logger->error("Add country {} fail for source", name, source);
    }
}

void HistoricalInfoPresenter::handleExtendContour(const std::string& name, 
                                                  const persistence::Coordinate& coordinate)
{
    if (!dynamicInfoModel.extendContour(source, dynamicInfoModel.getCurrentYear(), name, coordinate)) {
        logger->error("Extend country {} contour fail for source", name, source);
    }
}

void HistoricalInfoPresenter::handleAddCity(const std::string& name, const persistence::Coordinate& coordinate)
{
    if (!dynamicInfoModel.addCity(source, dynamicInfoModel.getCurrentYear(), persistence::City{name, coordinate})) {
        logger->error("Add city {} fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleRemoveCountry(const std::string& name)
{
    if (!dynamicInfoModel.removeCountry(source, dynamicInfoModel.getCurrentYear(), name)) {
        logger->error("Remove country {} fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleRemoveCity(const std::string& name)
{
    if (!dynamicInfoModel.removeCity(source, dynamicInfoModel.getCurrentYear(), name)) {
        logger->error("Remove city {} fail for source {}", name, source);
    }
}

std::string HistoricalInfoPresenter::handleGetNote() const noexcept
{
    if (auto note = dynamicInfoModel.getNote(source, dynamicInfoModel.getCurrentYear()); note) {
        return note.value();
    }

    return std::string{};
}

void HistoricalInfoPresenter::handleUpdateNote(const std::string& text)
{
    if (!dynamicInfoModel.addNote(source, dynamicInfoModel.getCurrentYear(), text)) {
        logger->error("Update note fail for source {}", source);
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
    return dynamicInfoModel.getCountryList(source, dynamicInfoModel.getCurrentYear());
}

std::list<persistence::Coordinate> HistoricalInfoPresenter::handleRequestContour(const std::string& name) const
{
    return dynamicInfoModel.getContour(source, dynamicInfoModel.getCurrentYear(), name);
}

void HistoricalInfoPresenter::handleUpdateContour(const std::string& name, int idx, const persistence::Coordinate& coordinate)
{
    if (!dynamicInfoModel.updateContour(source, dynamicInfoModel.getCurrentYear(), name, idx, coordinate)) {
        logger->error("Update country {} contour fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleDeleteFromContour(const std::string& name, int idx)
{
    if (!dynamicInfoModel.delectFromContour(source, dynamicInfoModel.getCurrentYear(), name, idx)) {
        logger->error("Delect idx {} from country {} contour fail for source {}", idx, name, source);
    }
}

std::vector<std::string> HistoricalInfoPresenter::handleRequestCityList() const
{
    return dynamicInfoModel.getCityList(source, dynamicInfoModel.getCurrentYear());
}

std::optional<persistence::Coordinate> HistoricalInfoPresenter::handleRequestCityCoordinate(const std::string& name) const
{
    return dynamicInfoModel.getCityCoord(source, dynamicInfoModel.getCurrentYear(), name);
}

void HistoricalInfoPresenter::handleUpdateCityCoordinate(const std::string& name, const persistence::Coordinate& coord)
{   
    if (!dynamicInfoModel.updateCityCoord(source, dynamicInfoModel.getCurrentYear(), name, coord)) {
        logger->error("Update city {} coordinate fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleClearHistoricalInfo()
{
    dynamicInfoModel.removeHistoricalInfoFromSource(source);
}
}