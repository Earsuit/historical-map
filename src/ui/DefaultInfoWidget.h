#ifndef SRC_UI_DEFAULT_INFO_WIDGET_H
#define SRC_UI_DEFAULT_INFO_WIDGET_H

#include "src/presentation/HistoricalInfoPresenter.h"
#include "src/presentation/DatabaseSaverPresenter.h"
#include "src/presentation/DatabaseYearPresenter.h"
#include "src/presentation/Util.h"
#include "src/logger/Util.h"
#include "src/ui/IInfoWidget.h"

#include "spdlog/spdlog.h"

#include <string>
#include <memory>
#include <map>

namespace ui {
class DefaultInfoWidget: public IInfoWidget {
public:
    DefaultInfoWidget(): 
        logger{spdlog::get(logger::LOGGER_NAME)}, 
        infoPresenter{presentation::DEFAULT_HISTORICAL_INFO_SOURCE},
        databaseSaverPresenter{presentation::DEFAULT_HISTORICAL_INFO_SOURCE}
    {
    }

    virtual bool complete() const noexcept override { return false; };
    virtual void paint() override;

private:
    std::shared_ptr<spdlog::logger> logger;
    presentation::HistoricalInfoPresenter infoPresenter;
    presentation::DatabaseSaverPresenter databaseSaverPresenter;
    presentation::DatabaseYearPresenter yearPresenter;
    int startYear;
    int endYear;
    int currentYear;
    std::string countryName;
    std::map<std::string, std::pair<std::string, std::string>> countryNewCoordinateCache;
    std::string newCityName, newCityLatitude, newCityLongitude;

    void displayCountryInfos();
    void displayCityInfos();
    void displayNote();
    void displayYearControlSection();
    void saveInfo(int year);
    void saveInfoRange(int startYear, int endYear);
    void savePopupWindow();
    void saveProgressPopUp();
    void displayCountry(const std::string& name);
    void displayCity(const std::string& name);
    persistence::Coordinate displayCoordinate(const std::string& uniqueId, const persistence::Coordinate& coord);
};

}

#endif
