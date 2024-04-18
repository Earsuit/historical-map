#ifndef SRC_UI_EXPORTWIDGET
#define SRC_UI_EXPORTWIDGET

#include "src/ui/IInfoWidget.h"
#include "src/persistence/DatabaseManager.h"
#include "src/persistence/exporterImporter/ExportManager.h"
#include "src/persistence/Selector.h"
#include "src/logger/Util.h"
#include "src/util/Generator.h"

#include "spdlog/spdlog.h"
#include "imgui.h"

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
    virtual std::optional<persistence::Coordinate> getHovered() const noexcept override;

private:
    std::shared_ptr<spdlog::logger> logger;
    persistence::DatabaseManager& database;
    persistence::ExportManager exporter;
    persistence::Selector selector;
    std::future<tl::expected<void, persistence::Error>> exportTask;
    bool exportComplete = false;
    std::optional<persistence::Coordinate> hovered;
    std::shared_ptr<const persistence::Data> cache;
    std::map<int, bool> selectAlls;
    bool isComplete = false;
    std::string exportFormat;
    std::string errorMsg;
    bool exportFailPopup = false;
    int currentYear;
    int startYear;
    int endYear;
    util::Generator<int> generator;
    bool processMultiYearSelection = false;

    virtual int historyInfo(int year) override;

    void handleCountryInfo(bool selectAll);
    void handleCityInfo(bool selectAll);
    void handleNote(bool selectAll);
    void checkExportProgress();
    void selectMultiYears();
    util::Generator<int> multiYearsSelectionGenerator(int start, int end);

    template<typename T>
    void checkbox(const T& item, bool& tick)
    {
        ImGui::Checkbox(("##" + item.name).c_str(), &tick);
    }

    template<typename T>
    requires std::is_same_v<T, persistence::Note>
    void checkbox(const T& item, bool& tick)
    {
        ImGui::Checkbox("##note", &tick);
    }

    template<typename T>
    void select(const T& item, bool selectAll)
    {
        bool tick = selectAll || selector.isSelected(item, currentYear);

        checkbox(item, tick);

        if (tick) {
            selector.select(item, cache);
        } else {
            selector.deselect(item, cache);
        }

        selectAlls[currentYear] = selectAlls[currentYear] & tick;
    }
};
}

#endif /* SRC_UI_EXPORTWIDGET */