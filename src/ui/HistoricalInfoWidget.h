#ifndef SRC_UI_HISTORICAL_INFO_WIDGET_H
#define SRC_UI_HISTORICAL_INFO_WIDGET_H

#include "src/persistence/DatabaseManager.h"
#include "src/persistence/Data.h"
#include "src/logger/Util.h"
#include "src/ui/CountryInfoWidget.h"
#include "src/ui/IInfoWidget.h"

#include "spdlog/spdlog.h"

#include <string>
#include <queue>
#include <optional>
#include <list>
#include <memory>
#include <utility>

namespace ui {
constexpr int QIN_DYNASTY = -221;

class HistoricalInfoWidget: public IInfoWidget {
public:
    HistoricalInfoWidget(): 
        IInfoWidget{QIN_DYNASTY},
        logger{spdlog::get(logger::LOGGER_NAME)}, 
        database{persistence::DatabaseManager::getInstance()},
        remove{std::make_shared<persistence::Data>(QIN_DYNASTY)}
    {
    }

    std::vector<HistoricalInfo> getInfo() override;
    void drawRightClickMenu(float longitude, float latitude) override;
    bool complete() override;

private:
    std::shared_ptr<spdlog::logger> logger;
    persistence::DatabaseManager& database;
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

    void countryInfo();
    void cityInfo();
    void displayNote();
    void saveInfo(int year);
    void saveInfoRange(int startYear, int endYear);
    void savePopupWindow();
    void displaySaveProgress();

    void historyInfo() override;
};

}

#endif
