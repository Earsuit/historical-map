#ifndef SRC_UI_HISTORICAL_INFO_WIDGET_H
#define SRC_UI_HISTORICAL_INFO_WIDGET_H

#include "src/persistence/DatabaseManager.h"
#include "src/persistence/Data.h"
#include "src/logger/Util.h"
#include "src/ui/CountryInfoWidget.h"

#include "spdlog/spdlog.h"

#include <string>
#include <queue>
#include <optional>
#include <list>
#include <memory>
#include <utility>

namespace ui {
constexpr auto HISTORICAL_INFO_WIDGET_NAME = "Historical Info";
constexpr int QIN_DYNASTY = -221;

class HistoricalInfoWidget {
public:
    HistoricalInfoWidget(): 
        logger{spdlog::get(logger::LOGGER_NAME)}, 
        remove{std::make_shared<persistence::Data>(QIN_DYNASTY)}
    {
    }

    void paint();

    void drawRightClickMenu(float longitude, float latitude);

    std::pair<std::shared_ptr<persistence::Data>, std::optional<persistence::Coordinate>> getInfo();

private:
    std::shared_ptr<spdlog::logger> logger;
    persistence::DatabaseManager database;
    int year = QIN_DYNASTY;
    std::shared_ptr<persistence::Data> cache;
    std::shared_ptr<persistence::Data> remove;
    std::optional<persistence::Coordinate> selected;
    std::list<CountryInfoWidget> countryInfoWidgets;
    std::string newCityName;
    std::string cityLongitude;
    std::string cityLatitude;
    int startYear = year;
    int endYear = year;
    float totalWorkLoad = 0;

    void historyInfo();
    void countryInfo();
    void cityInfo();
    void displayNote();
    void saveInfo(int year);
    void saveInfoRange(int startYear, int endYear);
    void savePopupWindow();
    void displaySaveProgress();
};

}

#endif
