#ifndef SRC_UI_EXPORTWIDGET
#define SRC_UI_EXPORTWIDGET

#include "src/ui/IInfoWidget.h"
#include "src/ui/CountryInfoWidget.h"
#include "src/persistence/DatabaseManager.h"
#include "src/persistence/exporterImporter/ExportManager.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <future>

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
    std::shared_ptr<spdlog::logger> logger;
    persistence::DatabaseManager& database;
    persistence::ExportManager exporter;
    std::future<tl::expected<void, persistence::Error>> exportTask;
    std::optional<persistence::Coordinate> hovered;
    std::shared_ptr<const persistence::Data> cache;
    std::map<int, bool> selectAlls;
    std::vector<CountryInfoWidget<decltype(persistence::Data::countries)::const_iterator>> countryInfoWidgets;
    bool isComplete = false;
    std::string exportFormat;
    std::string errorMsg;
    bool exportFailPopup = false;

    void historyInfo() override;

    void paintCountryInfo(bool selectAll);
    void paintCityInfo(bool selectAll);
    void paintNote(bool selectAll);
    void checkExportProgress();

    template<typename T>
    void checkbox(const T& item, bool& tick)
    {
        ImGui::Checkbox(("##" + item.name).c_str(), &tick);
    }

    template<>
    void checkbox(const persistence::Note& item, bool& tick)
    {
        ImGui::Checkbox("##note", &tick);
    }

    template<typename T>
    void select(const T& item, bool selectAll)
    {
        bool tick = selectAll || exporter.isSelected(item, year);

        checkbox(item, tick);

        if (tick) {
            exporter.select(item, cache);
        } else {
            exporter.deselect(item, cache);
        }

        selectAlls[year] = selectAlls[year] & tick;
    }
};
}

#endif /* SRC_UI_EXPORTWIDGET */