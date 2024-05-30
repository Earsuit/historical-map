#include "src/presentation/HistoricalInfoPresenter.h"

namespace presentation {
HistoricalInfoPresenter::HistoricalInfoPresenter(const std::string& source):
    logger{spdlog::get(logger::LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    dynamicInfoModel{model::DynamicInfoModel::getInstance()},
    source{source}
{
    util::signal::connect(&dynamicInfoModel, 
                          &model::DynamicInfoModel::onCountryUpdate, 
                          this, 
                          &HistoricalInfoPresenter::onCountryUpdate);
    util::signal::connect(&dynamicInfoModel,
                          &model::DynamicInfoModel::onCityUpdate,
                          this,
                          &HistoricalInfoPresenter::onCityUpdate);
    util::signal::connect(&dynamicInfoModel,
                          &model::DynamicInfoModel::onNoteUpdate,
                          this,
                          &HistoricalInfoPresenter::onNoteUpdate);
}

HistoricalInfoPresenter::~HistoricalInfoPresenter()
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
}

void HistoricalInfoPresenter::handleAddCountry(const std::string& name)
{
    if (!dynamicInfoModel.addCountry(source, databaseModel.getYear(),name)) {
        logger->error("Add country {} fail for source", name, source);
    }
}

void HistoricalInfoPresenter::handleExtendContour(const std::string& name, 
                                                  const persistence::Coordinate& coordinate)
{
    if (!dynamicInfoModel.extendContour(source, databaseModel.getYear(), name, coordinate)) {
        logger->error("Extend country {} contour fail for source", name, source);
    }
}

void HistoricalInfoPresenter::handleAddCity(const std::string& name, const persistence::Coordinate& coordinate)
{
    if (!dynamicInfoModel.addCity(source, databaseModel.getYear(), persistence::City{name, coordinate})) {
        logger->error("Add city {} fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleRemoveCountry(const std::string& name)
{
    if (!dynamicInfoModel.removeCountry(source, databaseModel.getYear(), name)) {
        logger->error("Remove country {} fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleRemoveCity(const std::string& name)
{
    if (!dynamicInfoModel.removeCity(source, databaseModel.getYear(), name)) {
        logger->error("Remove city {} fail for source {}", name, source);
    }
}

std::string HistoricalInfoPresenter::handleGetNote() const noexcept
{
    if (auto note = dynamicInfoModel.getNote(source, databaseModel.getYear()); note) {
        return note.value();
    }

    return std::string{};
}

void HistoricalInfoPresenter::handleUpdateNote(const std::string& text)
{
    if (!dynamicInfoModel.addNote(source, databaseModel.getYear(), text)) {
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
    return dynamicInfoModel.getCountryList(source, databaseModel.getYear());
}

std::list<persistence::Coordinate> HistoricalInfoPresenter::handleRequestContour(const std::string& name) const
{
    return dynamicInfoModel.getContour(source, databaseModel.getYear(), name);
}

void HistoricalInfoPresenter::handleUpdateContour(const std::string& name, int idx, const persistence::Coordinate& coordinate)
{
    if (!dynamicInfoModel.updateContour(source, databaseModel.getYear(), name, idx, coordinate)) {
        logger->error("Update country {} contour fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleDeleteFromContour(const std::string& name, int idx)
{
    if (!dynamicInfoModel.delectFromContour(source, databaseModel.getYear(), name, idx)) {
        logger->error("Delect idx {} from country {} contour fail for source {}", idx, name, source);
    }
}

std::vector<std::string> HistoricalInfoPresenter::handleRequestCityList() const
{
    return dynamicInfoModel.getCityList(source, databaseModel.getYear());
}

std::optional<persistence::Coordinate> HistoricalInfoPresenter::handleRequestCityCoordinate(const std::string& name) const
{
    return dynamicInfoModel.getCityCoord(source, databaseModel.getYear(), name);
}

void HistoricalInfoPresenter::handleUpdateCityCoordinate(const std::string& name, const persistence::Coordinate& coord)
{   
    if (!dynamicInfoModel.updateCityCoord(source, databaseModel.getYear(), name, coord)) {
        logger->error("Update city {} coordinate fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleClearHistoricalInfo()
{
    dynamicInfoModel.removeHistoricalInfoFromSource(source, databaseModel.getYear());
}

bool HistoricalInfoPresenter::varifySignal(const std::string& source, int year) const noexcept
{
    return source == this->source && year == databaseModel.getYear();
}

void HistoricalInfoPresenter::onCountryUpdate(const std::string& source, int year)
{
    if (varifySignal(source, year)) {
        logger->debug("HistoricalInfoPresenter onCountryUpdate for source {} at year {}", source, year);
        setCountriesUpdated();
    }
}

void HistoricalInfoPresenter::onCityUpdate(const std::string& source, int year)
{
    if (varifySignal(source, year)) {
        logger->debug("HistoricalInfoPresenter onCityUpdate for source {} at year {}", source, year);
        setCityUpdated();
    }
}

void HistoricalInfoPresenter::onNoteUpdate(const std::string& source, int year)
{
    if (varifySignal(source, year)) {
        logger->debug("HistoricalInfoPresenter onNoteUpdate for source {} at year {}", source, year);
        setNoteUpdated();
    }
}
}