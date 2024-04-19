#include "src/ui/ExportWidget.h"
#include "src/ui/Util.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

#include "ImFileDialog.h"
#include "imgui.h"

namespace ui {
using namespace std::chrono_literals;

constexpr auto EXPORT_FORMAT_POPUP_NAME = "Export formats";
constexpr auto SAVE_DIALOG_KEY = "ExportDialog";
constexpr auto EXPORT_FAIL_POPUP_NAME = "Error";
constexpr auto EXPORT_PROGRESS_POPUP_NAME = "Exporting";
constexpr auto PROGRESS_BAR_SIZE = ImVec2{400, 0};
constexpr auto EXPORT_PROGRESS_DONE_BUTTON_LABEL = "Done";

void ExportWidget::buttons()
{
    if (ImGui::Button("Export as")) {
        ImGui::OpenPopup(EXPORT_FORMAT_POPUP_NAME);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        isComplete = true;
    }
}

int ExportWidget::overwriteYear(int year)
{
    currentYear = year;
    return currentYear;
}

void ExportWidget::updateInfo()
{
    if (!cache || cache->year != currentYear) {
        logger->debug("Load data of year {} from database.", currentYear);

        cache = database.load(currentYear);
    }
}

bool ExportWidget::cacheReady() const noexcept
{
    return cache && cache->year == currentYear;
}

void ExportWidget::doExportImport(const persistence::Selector& selector)
{
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
}

std::optional<HistoricalInfoPack> ExportWidget::getSelectableInfo() const
{
    return HistoricalInfoPack{cache, "Export"};
}

std::optional<HistoricalInfoPack> ExportWidget::getUnselectableInfo() const
{
    return std::nullopt;
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
        simpleProgressDisplayer(exporter.getExportProgress(),
                                EXPORT_PROGRESS_DONE_BUTTON_LABEL,
                                [exportComplete = this->exportComplete](){
                                    return exportComplete;
                                },
                                [this](){
                                    this->isComplete = true;
                                });

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal(EXPORT_FAIL_POPUP_NAME, &exportFailPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Failed to export: %s", errorMsg.c_str());
    }
}
}