#ifndef SRC_UI_HISTORICAL_INFO_WIDGET_H
#define SRC_UI_HISTORICAL_INFO_WIDGET_H

#include "src/presentation/HistoricalInfoWidgetInterface.h"
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
class HistoricalInfoWidget: public IInfoWidget, public presentation::HistoricalInfoWidgetInterface {
public:
    HistoricalInfoWidget(): 
        logger{spdlog::get(logger::LOGGER_NAME)}, 
        infoPresenter{*this, SOURCE},
        databaseAccessPresenter{SOURCE}
    {
    }

    virtual void displayCountry(const std::string& name) override;
    virtual void displayCity(const std::string& name) override;
    virtual persistence::Coordinate displayCoordinate(const persistence::Coordinate& coord) override;
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
    std::map<std::string, std::pair<std::string, std::string>> countryNewCoordinate;
    std::string newCityName, newCityLatitude, newCityLongitude;

    void displayCountryInfo();
    void displayCityInfo();
    void displayNote();
    void saveInfo(int year);
    void saveInfoRange(int startYear, int endYear);
    void savePopupWindow();
    void saveProgressPopUp();

    virtual void historyInfo(int year) override;
};

}

#endif
