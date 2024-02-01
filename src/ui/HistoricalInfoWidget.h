#ifndef SRC_UI_HISTORICAL_INFO_WIDGET_H
#define SRC_UI_HISTORICAL_INFO_WIDGET_H

#include "src/persistence/PersistenceManager.h"
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
    persistence::PersistenceManager persistence;
    int year = QIN_DYNASTY;
    bool yearLock = false;
    std::shared_ptr<persistence::Data> cache;
    std::shared_ptr<persistence::Data> remove;
    std::optional<persistence::Coordinate> selected;
    std::list<CountryInfoWidget> countryInfoWidgets;

    void historyInfo();
    void countryInfo();
};

}

#endif
