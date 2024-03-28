#include "src/ui/ExportWidget.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

#include "ImFileDialog.h"

namespace ui {
constexpr auto EXPORT_FORMAT_POPUP_NAME = "Export formats";
constexpr auto SAVE_DIALOG_KEY = "ExportDialog";
constexpr auto SAVE_CONFIRM_POPUP_NAME = "Confirmation";

void ExportWidget::historyInfo()
{
    selected = std::nullopt;

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

    bool tick = false;

    if (cache) {
        tick = toBeExported.contains(cache);
    }

    ImGui::Checkbox("Add", &tick);

    if (cache) {
        if (tick) {
            toBeExported.insert(cache);
        } else {
            toBeExported.erase(cache);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Export as")) {
        ImGui::OpenPopup(EXPORT_FORMAT_POPUP_NAME);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        isComplete = true;
    }

    if (ImGui::BeginPopup(EXPORT_FORMAT_POPUP_NAME)) {
        for (auto& format : persistence::ExporterImporterFactory::getInstance().supportedFormat()) {
            if(ImGui::Selectable(format.c_str())) {
                exportFormat = format;
                ifd::FileDialog::getInstance().save(SAVE_DIALOG_KEY, "Export historical info", "*." + exportFormat + " {." + exportFormat +"}");
            }
        }

        ImGui::EndPopup();
    }

    if (ifd::FileDialog::getInstance().isDone(SAVE_DIALOG_KEY)) {
        if (ifd::FileDialog::getInstance().hasResult()) {
            result = ifd::FileDialog::getInstance().getResult().u8string();
        } 
        ifd::FileDialog::getInstance().close();
    }

    processResult();

    if (cache) {
        ImGui::SeparatorText("Countries");
        paintCountryInfo();

        ImGui::SeparatorText("Cities");
        paintCityInfo();

        ImGui::SeparatorText("Note");
        paintNote();
    }
}

void ExportWidget::paintCountryInfo()
{
    for (auto& countryInfoWidget : countryInfoWidgets) {
        if (ImGui::TreeNode((countryInfoWidget.getName() + "##country").c_str())) {
            ImGui::SeparatorText(countryInfoWidget.getName().c_str());
            countryInfoWidget.paint(selected);

            ImGui::TreePop();
            ImGui::Spacing();
        }
    }
}

void ExportWidget::paintCityInfo()
{
    for (auto& city : cache->cities) {
        bool hovered = false;

        if (ImGui::TreeNode((city.name + "##city").c_str())) {
            ImGui::Text("latitude %.2f", city.coordinate.latitude);
            hovered |= ImGui::IsItemHovered();
            ImGui::SameLine();
            ImGui::Text("longitude %.2f", city.coordinate.longitude);
            hovered |= ImGui::IsItemHovered();

            if (hovered) {
                selected = city.coordinate;
            }

            ImGui::TreePop();
            ImGui::Spacing();
        }
    }
}

void ExportWidget::paintNote()
{
    ImGui::TextUnformatted(cache->note.text.c_str(), cache->note.text.c_str() + cache->note.text.size());
}

std::vector<HistoricalInfo> ExportWidget::getInfo()
{
    return {std::make_tuple("Export", cache, selected)};
}

bool ExportWidget::complete()
{
    return isComplete;
}

void ExportWidget::processResult()
{
    if (result) {
        if (!exporter) {
            if(auto ret = persistence::ExporterImporterFactory::getInstance().createExporter(exportFormat); ret) {
                exporter = std::move(*ret);
                for (auto info : toBeExported) {
                    exporter->insert(*info);
                }

                if (auto ret = exporter->writeToFile(*result, false); !ret) {
                    // fail to write to a file
                    ImGui::OpenPopup(SAVE_CONFIRM_POPUP_NAME);
                    confirmPopupOpen = true;
                } else {
                    isComplete = true;
                }
            }
        }

        if (exporter) {
            if (ImGui::BeginPopupModal(SAVE_CONFIRM_POPUP_NAME, &confirmPopupOpen, ImGuiWindowFlags_AlwaysAutoResize) && exporter) {
                ImGui::Text("File already exists, do you want to overwrite it?");

                if (ImGui::Button("Yes")) {
                    if (exporter->writeToFile(*result, true)) {
                        confirmPopupOpen = false;
                        isComplete = true;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("No")) {
                    confirmPopupOpen = false;
                }

                ImGui::EndPopup();
            }

            if (!confirmPopupOpen) {
                exporter.release();
                result.reset();
            }
        }
    }
}
}