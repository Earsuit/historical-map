#ifndef SRC_UI_EXPORTWIDGET
#define SRC_UI_EXPORTWIDGET

#include "src/ui/IInfoWidget.h"
#include "src/ui/CountryInfoWidget.h"
#include "src/persistence/DatabaseManager.h"
#include "src/persistence/exporterImporter/IExporterImporter.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <memory>
#include <vector>
#include <map>
#include <set>
#include <string>

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
    struct CompareYear
    {
        bool operator()(const auto& lhs, const auto& rhs) const
        {
            return lhs->year < rhs->year;
        }
    };

    struct CompareString
    {
        bool operator()(const auto& lhs, const auto& rhs) const
        {
            return std::hash<std::string>{}(lhs) < std::hash<std::string>{}(rhs);
        }
    };

    struct Selected
    {
        std::set<std::string, CompareString> countries;
        std::set<std::string, CompareString> cities;
        bool note;
    };

    std::shared_ptr<spdlog::logger> logger;
    persistence::DatabaseManager& database;
    std::optional<persistence::Coordinate> hovered;
    std::shared_ptr<const persistence::Data> cache;
    std::map<int, bool> selectAlls;
    std::map<std::shared_ptr<const persistence::Data>, Selected, CompareYear> toBeExported;
    std::vector<CountryInfoWidget<decltype(persistence::Data::countries)::const_iterator>> countryInfoWidgets;
    bool isComplete = false;
    std::string exportFormat;
    std::unique_ptr<persistence::IExporter> exporter;
    std::optional<std::string> result;
    bool confirmPopupOpen = true;

    void historyInfo() override;

    void paintCountryInfo(bool selectAll);
    void paintCityInfo(bool selectAll);
    void paintNote(bool selectAll);
    void processResult();
    void selectCountry(const std::string& name, bool selectAll);
    void selectCity(const std::string& name, bool selectAll);
    void selectNote(bool selectAll);
    void doSelect(const std::string& name, std::set<std::string, CompareString>& container, bool selectAll);
};
}

#endif /* SRC_UI_EXPORTWIDGET */