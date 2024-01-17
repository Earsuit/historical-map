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

namespace ui {
constexpr auto HISTORICAL_INFO_WIDGET_NAME = "Historical Info";
constexpr int QIN_DYNASTY = -221;
constexpr int DEFAULT_CACHE_YEAR = 0;

class HistoricalInfoWidget {
public:
    HistoricalInfoWidget(): logger{spdlog::get(logger::LOGGER_NAME)} {}

    void paint();

private:
    std::shared_ptr<spdlog::logger> logger;
    persistence::PersistenceManager persistence;
    int year = QIN_DYNASTY;
    bool yearLock = false;
    persistence::Data cache{DEFAULT_CACHE_YEAR};
    std::optional<persistence::Coordinate> selected;
    std::list<CountryInfoWidget> countryInfoWidgets;

    void historyInfo();
    void countryInfo();
};

}

#endif
