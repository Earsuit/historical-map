#ifndef SRC_PRESENTATION_HISTORICAL_INFO_PRESENTER_H
#define SRC_PRESENTATION_HISTORICAL_INFO_PRESENTER_H

#include "src/model/CacheModel.h"
#include "src/model/DatabaseModel.h"
#include "src/util/Signal.h"
#include "src/util/Worker.h"
#include "src/logger/ModuleLogger.h"

#include <string>
#include <optional>

namespace presentation {
class HistoricalInfoPresenter {
public:
    HistoricalInfoPresenter(const std::string& source);
    ~HistoricalInfoPresenter();

    std::string handleGetNote() const noexcept;
    void handleUpdateNote(const std::string& text);
    void handleAddCountry(const std::string& name);
    void handleExtendContour(const std::string& name, const persistence::Coordinate& coordinate);
    void handleAddCity(const std::string& name, const persistence::Coordinate& coordinate);
    void handleRemoveCountry(const std::string& name);
    void handleRemoveCity(const std::string& name);
    void setHoveredCoord(const persistence::Coordinate& coordinate);
    void clearHoveredCoord();
    std::vector<std::string> handleRequestCountryList() const;
    std::list<persistence::Coordinate> handleRequestContour(const std::string& name) const;
    void handleUpdateContour(const std::string& name, int idx, const persistence::Coordinate& coordinate);
    void handleDeleteFromContour(const std::string& name, int idx);
    std::vector<std::string> handleRequestCityList() const;
    std::optional<persistence::Coordinate> handleRequestCityCoordinate(const std::string& name) const;
    void handleUpdateCityCoordinate(const std::string& name, const persistence::Coordinate& coord);
    void handleClearHistoricalInfo();
    bool handleCheckIsModified();

    util::signal::Signal<void()> setCountriesUpdated;
    util::signal::Signal<void()> setCityUpdated;
    util::signal::Signal<void()> setNoteUpdated;
    util::signal::Signal<void(bool)> setModificationState;

private:
    logger::ModuleLogger logger;
    model::DatabaseModel& databaseModel;
    model::CacheModel& cacheModel;
    std::string source;
    util::Worker<std::function<void()>> worker;

    bool varifySignal(const std::string& source, int year) const noexcept;
    void onCountryUpdate(const std::string& source, int year);
    void onCityUpdate(const std::string& source, int year);
    void onNoteUpdate(const std::string& source, int year);
    void onModificationChange(const std::string& source, int year, bool isModified);
};
}


#endif
