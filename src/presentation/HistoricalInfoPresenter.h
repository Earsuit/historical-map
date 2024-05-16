#ifndef SRC_PRESENTATION_HISTORICAL_INFO_PRESENTER_H
#define SRC_PRESENTATION_HISTORICAL_INFO_PRESENTER_H

#include "src/model/DynamicInfoModel.h"
#include "src/model/DatabaseModel.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <string>
#include <optional>

namespace presentation {
class HistoricalInfoPresenter {
public:
    HistoricalInfoPresenter(const std::string& source):
        logger{spdlog::get(logger::LOGGER_NAME)},
        databaseModel{model::DatabaseModel::getInstance()},
        dynamicInfoModel{model::DynamicInfoModel::getInstance()},
        source{source}
    {}

    std::optional<std::string> handleGetNote() const noexcept;
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

private:
    std::shared_ptr<spdlog::logger> logger;
    model::DatabaseModel& databaseModel;
    model::DynamicInfoModel& dynamicInfoModel;
    std::string source;
};
}


#endif
