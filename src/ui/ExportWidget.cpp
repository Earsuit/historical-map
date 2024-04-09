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
constexpr auto EXPORT_PROGRESS_BAR_SIZE = ImVec2{400, 0};
constexpr auto EXPORT_PROGRESS_DONE_BUTTON_LABEL = "Done";

void ExportWidget::historyInfo()
{
    hovered = std::nullopt;

    if (!cache || cache->year != year) {
        logger->debug("Load data of year {} from database.", year);

        cache.reset();

        countryInfoWidgets.clear();

        if (cache = database.load(year); cache) {
            countryInfoWidgets.reserve(cache->countries.size());
            for (auto it = cache->countries.cbegin(); it != cache->countries.cend(); it++) {
                countryInfoWidgets.emplace_back(it);
            }
        }
    }
    
    if (!selectAlls.contains(year)) {
        selectAlls[year] = false;
    }

    ImGui::Checkbox("Select all", &selectAlls[year]);

    ImGui::SameLine();
    if (ImGui::Button("Export as")) {
        ImGui::OpenPopup(EXPORT_FORMAT_POPUP_NAME);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        isComplete = true;
    }

    if (cache) {
        const bool selectAll = selectAlls[year];
        selectAlls[year] = true;    // reset to true so we can track if there all individual items are ticked

        ImGui::SeparatorText("Countries");
        paintCountryInfo(selectAll);

        ImGui::SeparatorText("Cities");
        paintCityInfo(selectAll);

        ImGui::SeparatorText("Note");
        paintNote(selectAll);
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
            exportTask = std::move(exporter.doExport(file, exportFormat, true));
            ImGui::OpenPopup(EXPORT_PROGRESS_POPUP_NAME);
        } 
        ifd::FileDialog::getInstance().close();
    }

    checkExportProgress();
}

void ExportWidget::paintCountryInfo(bool selectAll)
{
    for (auto& countryInfoWidget : countryInfoWidgets) {

        select(*countryInfoWidget.getCountryIterator(), selectAll);
        ImGui::SameLine();

        if (ImGui::TreeNode((countryInfoWidget.getName() + "##country").c_str())) {
            countryInfoWidget.paint(hovered);

            ImGui::TreePop();
            ImGui::Spacing();
        }
    }
}

void ExportWidget::paintCityInfo(bool selectAll)
{
    for (auto& city : cache->cities) {
        bool isHovered = false;

        select(city, selectAll);
        ImGui::SameLine();

        if (ImGui::TreeNode((city.name + "##city").c_str())) {
            ImGui::Text("latitude %.2f", city.coordinate.latitude);
            isHovered |= ImGui::IsItemHovered();
            ImGui::SameLine();
            ImGui::Text("longitude %.2f", city.coordinate.longitude);
            isHovered |= ImGui::IsItemHovered();

            if (isHovered) {
                hovered = city.coordinate;
            }

            ImGui::TreePop();
            ImGui::Spacing();
        }
    }
}

void ExportWidget::paintNote(bool selectAll)
{
    if (!cache->note.text.empty()) {
        select(cache->note, selectAll);
    }

    ImGui::TextUnformatted(cache->note.text.c_str(), cache->note.text.c_str() + cache->note.text.size());
}

std::vector<HistoricalInfo> ExportWidget::getInfo()
{
    return {std::make_tuple("Export", cache, hovered)};
}

bool ExportWidget::complete()
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
        ImGui::ProgressBar(exporter.getExportProgress(), EXPORT_PROGRESS_BAR_SIZE);

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
}