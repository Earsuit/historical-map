#ifndef SRC_UI_EXPORT_IMPORT_WIDGET_H
#define SRC_UI_EXPORT_IMPORT_WIDGET_H

#include "src/persistence/Data.h"
#include "src/persistence/Selector.h"
#include "src/ui/IInfoWidget.h"
#include "src/ui/Util.h"
#include "src/util/Generator.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"
#include "imgui.h"

#include <map>
#include <optional>
#include <memory>
#include <vector>
#include <type_traits>

namespace ui {
class ExportImportWidget: public IInfoWidget {
public:
    ExportImportWidget(int year):
        IInfoWidget{year},
        logger{spdlog::get(logger::LOGGER_NAME)}
    {}

    virtual std::vector<HistoricalInfoPack> getInfos() const override;
    virtual std::optional<persistence::Coordinate> getHovered() const noexcept override;

private:
    std::shared_ptr<spdlog::logger> logger;
    std::map<int, bool> selectAlls;
    persistence::Selector selector;
    int currentYear;
    int startYear;
    int endYear;
    bool processMultiYearSelection = false;
    util::Generator<int> generator;
    std::optional<persistence::Coordinate> hovered;

    virtual int historyInfo(int year) override final;

    virtual bool cacheReady() const noexcept = 0;
    virtual int overwriteYear(int year) = 0;
    virtual std::optional<HistoricalInfoPack> getSelectableInfo() const = 0;
    virtual std::optional<HistoricalInfoPack> getUnselectableInfo() const = 0;
    virtual void doExportImport(const persistence::Selector& selector) = 0;
    virtual void buttons() = 0;
    virtual void updateInfo() = 0;

    void selectMultiYears();
    util::Generator<int> multiYearsSelectionGenerator(int start, int end);
    void handleInfo(const std::optional<HistoricalInfoPack>& info, bool selectable);
    void handleCountryInfo(std::shared_ptr<const persistence::Data> info, bool selectAll);
    void handleCountryInfo(std::shared_ptr<const persistence::Data> info);
    void handleCityInfo(std::shared_ptr<const persistence::Data> info, bool selectAll);
    void handleCityInfo(std::shared_ptr<const persistence::Data> info);
    void handleNote(std::shared_ptr<const persistence::Data> info, bool selectAll);
    void handleNote(std::shared_ptr<const persistence::Data> info);

    template<typename T>
    requires (std::is_same_v<std::remove_cvref_t<T>, persistence::Country> || 
              std::is_same_v<std::remove_cvref_t<T>, persistence::City>)
    void paintTreeNote(T&& info)
    {
        if (ImGui::TreeNode((info.name + "##" + typeid(T).name()).c_str())) {
            if (const auto& ret = paintInfo(std::forward<T>(info)); ret) {
                hovered = ret;
            }

            ImGui::TreePop();
            ImGui::Spacing();
        }
    }

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
    void select(const T& item, 
                std::shared_ptr<const persistence::Data> info, 
                bool selectAll)
    {
        bool tick = selectAll || selector.isSelected(item, info->year);

        checkbox(item, tick);

        if (tick) {
            selector.select(item, info);
        } else {
            selector.deselect(item, info);
        }

        selectAlls[currentYear] = selectAlls[currentYear] & tick;
    }
};
}

#endif
