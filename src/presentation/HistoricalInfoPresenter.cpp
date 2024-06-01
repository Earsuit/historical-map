#include "src/presentation/HistoricalInfoPresenter.h"

namespace presentation {
HistoricalInfoPresenter::HistoricalInfoPresenter(const std::string& source):
    logger{spdlog::get(logger::LOGGER_NAME)},
    databaseModel{model::DatabaseModel::getInstance()},
    cacheModel{model::CacheModel::getInstance()},
    source{source}
{
    util::signal::connect(&cacheModel, 
                          &model::CacheModel::onCountryUpdate, 
                          this, 
                          &HistoricalInfoPresenter::onCountryUpdate);
    util::signal::connect(&cacheModel,
                          &model::CacheModel::onCityUpdate,
                          this,
                          &HistoricalInfoPresenter::onCityUpdate);
    util::signal::connect(&cacheModel,
                          &model::CacheModel::onNoteUpdate,
                          this,
                          &HistoricalInfoPresenter::onNoteUpdate);
}

HistoricalInfoPresenter::~HistoricalInfoPresenter()
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
}

void HistoricalInfoPresenter::handleAddCountry(const std::string& name)
{
    if (!cacheModel.addCountry(source, databaseModel.getYear(),name)) {
        logger->error("Add country {} fail for source", name, source);
    }
}

void HistoricalInfoPresenter::handleExtendContour(const std::string& name, 
                                                  const persistence::Coordinate& coordinate)
{
    if (!cacheModel.extendContour(source, databaseModel.getYear(), name, coordinate)) {
        logger->error("Extend country {} contour fail for source", name, source);
    }
}

void HistoricalInfoPresenter::handleAddCity(const std::string& name, const persistence::Coordinate& coordinate)
{
    if (!cacheModel.addCity(source, databaseModel.getYear(), persistence::City{name, coordinate})) {
        logger->error("Add city {} fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleRemoveCountry(const std::string& name)
{
    if (!cacheModel.removeCountry(source, databaseModel.getYear(), name)) {
        logger->error("Remove country {} fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleRemoveCity(const std::string& name)
{
    if (!cacheModel.removeCity(source, databaseModel.getYear(), name)) {
        logger->error("Remove city {} fail for source {}", name, source);
    }
}

std::string HistoricalInfoPresenter::handleGetNote() const noexcept
{
    if (auto note = cacheModel.getNote(source, databaseModel.getYear()); note) {
        return note.value();
    }

    return std::string{};
}

void HistoricalInfoPresenter::handleUpdateNote(const std::string& text)
{
    if (!cacheModel.addNote(source, databaseModel.getYear(), text)) {
        logger->error("Update note fail for source {}", source);
    }
}

void HistoricalInfoPresenter::setHoveredCoord(const persistence::Coordinate& coordinate)
{
    cacheModel.setHoveredCoord(coordinate);
}

void HistoricalInfoPresenter::clearHoveredCoord()
{
    cacheModel.clearHoveredCoord();
}

std::vector<std::string> HistoricalInfoPresenter::handleRequestCountryList() const
{
    return cacheModel.getCountryList(source, databaseModel.getYear());
}

std::list<persistence::Coordinate> HistoricalInfoPresenter::handleRequestContour(const std::string& name) const
{
    return cacheModel.getContour(source, databaseModel.getYear(), name);
}

void HistoricalInfoPresenter::handleUpdateContour(const std::string& name, int idx, const persistence::Coordinate& coordinate)
{
    if (!cacheModel.updateContour(source, databaseModel.getYear(), name, idx, coordinate)) {
        logger->error("Update country {} contour fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleDeleteFromContour(const std::string& name, int idx)
{
    if (!cacheModel.delectFromContour(source, databaseModel.getYear(), name, idx)) {
        logger->error("Delect idx {} from country {} contour fail for source {}", idx, name, source);
    }
}

std::vector<std::string> HistoricalInfoPresenter::handleRequestCityList() const
{
    return cacheModel.getCityList(source, databaseModel.getYear());
}

std::optional<persistence::Coordinate> HistoricalInfoPresenter::handleRequestCityCoordinate(const std::string& name) const
{
    return cacheModel.getCityCoord(source, databaseModel.getYear(), name);
}

void HistoricalInfoPresenter::handleUpdateCityCoordinate(const std::string& name, const persistence::Coordinate& coord)
{   
    if (!cacheModel.updateCityCoord(source, databaseModel.getYear(), name, coord)) {
        logger->error("Update city {} coordinate fail for source {}", name, source);
    }
}

void HistoricalInfoPresenter::handleClearHistoricalInfo()
{
    cacheModel.removeHistoricalInfoFromSource(source, databaseModel.getYear());
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