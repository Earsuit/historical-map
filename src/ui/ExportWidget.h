#ifndef SRC_UI_EXPORTWIDGET
#define SRC_UI_EXPORTWIDGET

#include "src/ui/IInfoWidget.h"
#include "src/ui/CountryInfoWidget.h"
#include "src/persistence/DatabaseManager.h"
#include "src/persistence/exporterImporter/IExporterImporter.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <memory>
#include <list>
#include <set>

namespace ui {
class ExportWidget: public IInfoWidget {
public:
    ExportWidget(int year):
        IInfoWidget{year},
        logger{spdlog::get(logger::LOGGER_NAME)}, 
        database{persistence::DatabaseManager::getInstance()}
    {
    }

    void drawRightClickMenu(float longitude, float latitude) override {};
    std::vector<HistoricalInfo> getInfo() override;
    bool complete() override;

private:
    struct DataCompare
    {
        bool operator()(std::shared_ptr<persistence::Data> lhs, std::shared_ptr<persistence::Data> rhs) const
        {
            return lhs->year < rhs->year;
        }
    };

    std::shared_ptr<spdlog::logger> logger;
    persistence::DatabaseManager& database;
    std::optional<persistence::Coordinate> selected;
    std::shared_ptr<persistence::Data> cache;
    std::set<std::shared_ptr<persistence::Data>, DataCompare> toBeExported;
    std::list<CountryInfoWidget> countryInfoWidgets;
    bool isComplete = false;
    std::string exportFormat;
    std::unique_ptr<persistence::IExporter> exporter;
    std::optional<std::string> result;
    bool confirmPopupOpen = true;

    void historyInfo() override;

    void paintCountryInfo();
    void paintCityInfo();
    void paintNote();
    void processResult();
};
}

#endif /* SRC_UI_EXPORTWIDGET */
