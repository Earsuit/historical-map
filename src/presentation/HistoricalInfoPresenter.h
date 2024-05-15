#ifndef SRC_PRESENTATION_HISTORICAL_INFO_PRESENTER_H
#define SRC_PRESENTATION_HISTORICAL_INFO_PRESENTER_H

#include "src/model/DynamicInfoModel.h"
#include "src/model/DatabaseModel.h"
#include "src/presentation/HistoricalInfoWidgetInterface.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <string>
#include <optional>

namespace presentation {
class HistoricalInfoPresenter {
public:
    HistoricalInfoPresenter(HistoricalInfoWidgetInterface& view, const std::string& source):
        logger{spdlog::get(logger::LOGGER_NAME)},
        view{view},
        databaseModel{model::DatabaseModel::getInstance()},
        dynamicInfoModel{model::DynamicInfoModel::getInstance()},
        source{source}
    {}

    void handleDisplayCountries();
    void handleDisplayCities();
    void handleDisplayCity(const std::string& name);
    void handleDisplayCountry(const std::string& name);
    std::optional<std::string> handleGetNote() const noexcept;
    void handleUpdateNote(const std::string& text);
    void handleAddCountry(const std::string& name);
    void handleExtendContour(const std::string& name, const persistence::Coordinate& coordinate);
    void handleAddCity(const std::string& name, const persistence::Coordinate& coordinate);
    void handleRemoveCountry(const std::string& name);
    void handleRemoveCity(const std::string& name);
    void setHoveredCoord(const persistence::Coordinate& coordinate);
    void clearHoveredCoord();

private:
    std::shared_ptr<spdlog::logger> logger;
    HistoricalInfoWidgetInterface& view;
    model::DatabaseModel& databaseModel;
    model::DynamicInfoModel& dynamicInfoModel;
    std::string source;
};
}


#endif
