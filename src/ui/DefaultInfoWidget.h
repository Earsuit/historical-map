#ifndef SRC_UI_DEFAULT_INFO_WIDGET_H
#define SRC_UI_DEFAULT_INFO_WIDGET_H

#include "src/presentation/HistoricalInfoPresenter.h"
#include "src/presentation/DatabaseAccessPresenter.h"
#include "src/logger/Util.h"
#include "src/ui/IInfoWidget.h"

#include "spdlog/spdlog.h"

#include <string>
#include <queue>
#include <optional>
#include <list>
#include <memory>
#include <utility>
#include <map>

namespace ui {
class DefaultInfoWidget: public IInfoWidget {
public:
    DefaultInfoWidget(): 
        logger{spdlog::get(logger::LOGGER_NAME)}, 
        infoPresenter{SOURCE},
        databaseAccessPresenter{SOURCE}
    {
    }

    virtual bool complete() const noexcept override { return false; };

private:
    constexpr static auto SOURCE = "Database";

    std::shared_ptr<spdlog::logger> logger;
    presentation::HistoricalInfoPresenter infoPresenter;
    presentation::DatabaseAccessPresenter databaseAccessPresenter;
    int startYear;
    int endYear;
    int currentYear;
    std::string countryName;
    std::map<std::string, std::pair<std::string, std::string>> countryNewCoordinateCache;
    std::string newCityName, newCityLatitude, newCityLongitude;

    void displayCountryInfos();
    void displayCityInfos();
    void displayNote();
    void saveInfo(int year);
    void saveInfoRange(int startYear, int endYear);
    void savePopupWindow();
    void saveProgressPopUp();
    void displayCountry(const std::string& name);
    void displayCity(const std::string& name);
    persistence::Coordinate displayCoordinate(const std::string& uniqueId, const persistence::Coordinate& coord);

    virtual void historyInfo(int year) override;
};

}

#endif
