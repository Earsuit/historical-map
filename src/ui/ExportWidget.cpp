#include "src/ui/ExportWidget.h"
#include "src/ui/Util.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

#include "ImFileDialog.h"
#include "imgui.h"

namespace ui {
using namespace std::chrono_literals;

constexpr auto EXPORT_FORMAT_POPUP_NAME = "Export formats";
constexpr auto SAVE_DIALOG_KEY = "ExportDialog";
constexpr auto SAVE_CONFIRM_POPUP_NAME = "Confirmation";
constexpr auto EXPORT_FAIL_POPUP_NAME = "Error";
constexpr auto EXPORT_PROGRESS_POPUP_NAME = "Exporting";
constexpr auto PROGRESS_BAR_SIZE = ImVec2{400, 0};
constexpr auto EXPORT_PROGRESS_DONE_BUTTON_LABEL = "Done";
constexpr auto SELECT_MULTIPLE_YEAR_POPUP_NAME = "Select multiple years";
constexpr auto PROCESS_MULTI_YEAR_SELECTION_POPUP_NAME = "Selecting";
constexpr auto SELECT_MULTI_YEAR_YEAR_CONSTRAINTS = "Start year must be less than end year.";

int ExportWidget::historyInfo(int year)
{
    currentYear = year;

    hovered = std::nullopt;
    
    if (!selectAlls.contains(currentYear)) {
        selectAlls[currentYear] = false;
    }

    if (ImGui::Checkbox("Select all", &selectAlls[currentYear]) && !selectAlls[currentYear]) {
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

    if (ImGui::Button("Export as")) {
        ImGui::OpenPopup(EXPORT_FORMAT_POPUP_NAME);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        isComplete = true;
    }

    if (!cache || cache->year != year) {
        logger->debug("Load data of year {} from database.", year);

        cache = database.load(year);
    }

    if (cache) {
        const bool selectAll = selectAlls[year];
        // reset to true if not empty year so we can track if all individual items are ticked
        selectAlls[year] = !(cache->countries.empty() && cache->cities.empty() && cache->note.text.empty());

        ImGui::SeparatorText("Countries");
        handleCountryInfo(selectAll);

        ImGui::SeparatorText("Cities");
        handleCityInfo(selectAll);

        ImGui::SeparatorText("Note");
        handleNote(selectAll);
    }

    if (ImGui::BeginPopup(EXPORT_FORMAT_POPUP_NAME)) {
        for (auto& format : exporter.supportedFormat()) {
            if(ImGui::Selectable(format.c_str())) {
                exportFormat = format;
                ImGui::CloseCurrentPopup();
                ifd::FileDialog::getInstance().save(SAVE_DIALOG_KEY, "Export historical info", "*." + exportFormat + " {." + exportFormat +"}");
            }
        }

        ImGui::EndPopup();
    }

    if (ifd::FileDialog::getInstance().isDone(SAVE_DIALOG_KEY)) {
        if (ifd::FileDialog::getInstance().hasResult()) {
            const std::string file = ifd::FileDialog::getInstance().getResult().u8string();
            exportTask = std::move(exporter.doExport(selector, file, exportFormat, true));
            ImGui::OpenPopup(EXPORT_PROGRESS_POPUP_NAME);
        } 
        ifd::FileDialog::getInstance().close();
    }

    checkExportProgress();

    return currentYear;
}

void ExportWidget::handleCountryInfo(bool selectAll)
{
    for (const auto& country : cache->countries) {

        select(country, selectAll);
        ImGui::SameLine();

        if (ImGui::TreeNode((country.name + "##country").c_str())) {
            if (const auto& ret = paintInfo(country); ret) {
                hovered = ret;
            }

            ImGui::TreePop();
            ImGui::Spacing();
        }
    }
}

void ExportWidget::handleCityInfo(bool selectAll)
{
    for (auto& city : cache->cities) {
        bool isHovered = false;

        select(city, selectAll);
        ImGui::SameLine();

        if (ImGui::TreeNode((city.name + "##city").c_str())) {
            if (const auto& ret = paintInfo(city); ret) {
                hovered = ret;
            }

            ImGui::TreePop();
            ImGui::Spacing();
        }
    }
}

void ExportWidget::handleNote(bool selectAll)
{
    if (!cache->note.text.empty()) {
        select(cache->note, selectAll);
    }

    paintInfo(cache->note);
}

std::vector<HistoricalInfoPack> ExportWidget::getInfos() const
{
    return {HistoricalInfoPack{cache, "Export"}};
}

bool ExportWidget::complete() const noexcept
{
    return isComplete;
}

void ExportWidget::checkExportProgress()
{
    if (exportTask.valid() && exportTask.wait_for(0s) == std::future_status::ready) {
        if (auto ret = exportTask.get(); ret) {
            exportComplete = true;
        } else {
            auto errorMsg = ret.error().msg;
            logger->error("Failed to export: " + errorMsg);

            ImGui::OpenPopup(EXPORT_FAIL_POPUP_NAME);
            exportFailPopup = true;
        }
    }

    if (ImGui::BeginPopupModal(EXPORT_PROGRESS_POPUP_NAME, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::ProgressBar(exporter.getExportProgress(), PROGRESS_BAR_SIZE);

        alignForWidth(ImGui::CalcTextSize(EXPORT_PROGRESS_DONE_BUTTON_LABEL).x);

        if(!exportComplete) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button(EXPORT_PROGRESS_DONE_BUTTON_LABEL)) {
            isComplete = true;
            ImGui::CloseCurrentPopup();
        }
        if(!exportComplete) {
            ImGui::EndDisabled();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(EXPORT_FAIL_POPUP_NAME, &exportFailPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to export: %s", errorMsg.c_str());
    }
}

void ExportWidget::selectMultiYears()
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

        if (cache && cache->year == currentYear) {
            processMultiYearSelection  = generator.next();
            currentYear = generator.getValue();
            selectAlls[currentYear] = true;
        }

        ImGui::EndPopup();
    }
}

util::Generator<int> ExportWidget::multiYearsSelectionGenerator(int start, int end)
{
    for (int y = start; y <= end; y++) {
        co_yield y;
    }
}

std::optional<persistence::Coordinate> ExportWidget::getHovered() const noexcept
{
    return hovered;
}
}