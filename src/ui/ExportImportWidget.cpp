#include "src/ui/ExportImportWidget.h"
#include "src/ui/Util.h"

#include "imgui.h"

namespace ui {
constexpr auto SELECT_MULTIPLE_YEAR_POPUP_NAME = "Select multiple years";
constexpr auto PROCESS_MULTI_YEAR_SELECTION_POPUP_NAME = "Selecting";
constexpr auto SELECT_MULTI_YEAR_YEAR_CONSTRAINTS = "Start year must be less than end year.";
constexpr auto PROGRESS_BAR_SIZE = ImVec2{400, 0};
constexpr auto COUNTRY_SEPARATOR = "Countries";
constexpr auto CITY_SEPARATOR = "Cities";
constexpr auto NOTE_SEPARATOR = "Note";

int ExportImportWidget::historyInfo(int year)
{
    currentYear = overwriteYear(year);

    if (!selectAlls.contains(currentYear)) {
        selectAlls[year] = false;
    }

    if (ImGui::Checkbox("Select all", &selectAlls[currentYear]) && !selectAlls[currentYear]) {
        logger->debug("Clear all selections from year {}", currentYear);
        selector.clear(currentYear);
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup(SELECT_MULTIPLE_YEAR_POPUP_NAME);
        startYear = currentYear;
        endYear = currentYear;
    }
    ImGui::SameLine();
    helpMarker("Right click to select multiple years");

    selectMultiYears();

    buttons();

    updateInfo();

    hovered = std::nullopt;

    handleInfo(getSelectableInfo(), true);
    handleInfo(getUnselectableInfo(), false);
    
    doExportImport(selector);

    return currentYear;
}

void ExportImportWidget::selectMultiYears()
{
    if (ImGui::BeginPopup(SELECT_MULTIPLE_YEAR_POPUP_NAME)) {
        ImGui::SeparatorText(SELECT_MULTIPLE_YEAR_POPUP_NAME);
        ImGui::InputInt("Start", &startYear);
        ImGui::InputInt("End", &endYear);
        helpMarker(SELECT_MULTI_YEAR_YEAR_CONSTRAINTS);
        
        if (ImGui::Button("Select")) {
            if (startYear < endYear) {
                generator = multiYearsSelectionGenerator(startYear, endYear);
                processMultiYearSelection = true;
                ImGui::CloseCurrentPopup();
            } else {
                logger->error(SELECT_MULTI_YEAR_YEAR_CONSTRAINTS);
            }
        }
        
        ImGui::EndPopup();

        if (processMultiYearSelection) {
            ImGui::OpenPopup(PROCESS_MULTI_YEAR_SELECTION_POPUP_NAME);
        }
    }

    if (ImGui::BeginPopupModal(PROCESS_MULTI_YEAR_SELECTION_POPUP_NAME, &processMultiYearSelection, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::ProgressBar(static_cast<float>(currentYear - startYear) / (endYear - startYear), PROGRESS_BAR_SIZE);
        ImGui::Text("Processing year %d", currentYear);

        // only move to the next one when the cache is ready,
        // so the map plot will change the same time
        if (cacheReady()) {
            processMultiYearSelection  = generator.next();
            currentYear = generator.getValue();
            selectAlls[currentYear] = true;
        }

        ImGui::EndPopup();
    }
}

util::Generator<int> ExportImportWidget::multiYearsSelectionGenerator(int start, int end)
{
    for (int y = start; y <= end; y++) {
        co_yield y;
    }
}

void ExportImportWidget::handleInfo(const std::optional<HistoricalInfoPack>& info, bool selectable)
{
    if (info) {
        std::visit([this, 
                    selectable,
                    source = info->source](auto&& info){
            if (info) {
                ImGui::SeparatorText(source.c_str());

                if (selectable) {
                    bool selectAll = selectAlls[currentYear];;
                    // reset to true if not empty year so we can track if all individual items are ticked
                    selectAlls[currentYear] = !(info->countries.empty() && info->cities.empty() && info->note.text.empty());

                    ImGui::SeparatorText(COUNTRY_SEPARATOR);
                    this->handleCountryInfo(info, selectAll);

                    ImGui::SeparatorText(CITY_SEPARATOR);
                    this->handleCityInfo(info, selectAll);

                    ImGui::SeparatorText(NOTE_SEPARATOR);
                    this->handleNote(info, selectAll);
                } else {
                    ImGui::SeparatorText(COUNTRY_SEPARATOR);
                    this->handleCountryInfo(info);

                    ImGui::SeparatorText(CITY_SEPARATOR);
                    this->handleCityInfo(info);

                    ImGui::SeparatorText(NOTE_SEPARATOR);
                    this->handleNote(info);
                }
            }
        },
        info->info);
    }
}

void ExportImportWidget::handleCountryInfo(std::shared_ptr<const persistence::Data> info, bool selectAll)
{
    for (const auto& country : info->countries) {

        select(country, info, selectAll);
        ImGui::SameLine();

        paintTreeNote(country);
    }
}

void ExportImportWidget::handleCountryInfo(std::shared_ptr<const persistence::Data> info)
{
    for (const auto& country : info->countries) {
        paintTreeNote(country);
    }
}

void ExportImportWidget::handleCityInfo(std::shared_ptr<const persistence::Data> info, bool selectAll)
{
    for (auto& city : info->cities) {
        select(city, info, selectAll);
        ImGui::SameLine();

        paintTreeNote(city);
    }
}

void ExportImportWidget::handleCityInfo(std::shared_ptr<const persistence::Data> info)
{
    for (auto& city : info->cities) {
        paintTreeNote(city);
    }
}

void ExportImportWidget::handleNote(std::shared_ptr<const persistence::Data> info, bool selectAll)
{
    if (!info->note.text.empty()) {
        select(info->note, info, selectAll);
    }

    paintInfo(info->note);
}

void ExportImportWidget::handleNote(std::shared_ptr<const persistence::Data> info)
{
    paintInfo(info->note);
}

std::optional<persistence::Coordinate> ExportImportWidget::getHovered() const noexcept
{
    return hovered;
}

std::vector<HistoricalInfoPack> ExportImportWidget::getInfos() const
{
    std::vector<HistoricalInfoPack> out;

    if (const auto& selectable = getSelectableInfo(); selectable) {
        out.emplace_back(*selectable);
    }

    if (const auto& unselectable = getUnselectableInfo(); unselectable) {
        out.emplace_back(*unselectable);
    }

    return out;
}
}