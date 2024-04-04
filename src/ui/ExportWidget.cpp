#include "src/ui/ExportWidget.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

#include "ImFileDialog.h"

namespace ui {
constexpr auto EXPORT_FORMAT_POPUP_NAME = "Export formats";
constexpr auto SAVE_DIALOG_KEY = "ExportDialog";
constexpr auto SAVE_CONFIRM_POPUP_NAME = "Confirmation";

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

    if (ImGui::Checkbox("Select all", &selectAlls[year])) {
        if (!selectAlls[year]) {
            // select all is unticked, deselect all
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

    if (cache) {
        const bool selectAll = selectAlls[year];
        selectAlls[year] = true;    // reset to true so we can track if there all individual items are ticked

        if (!toBeExported.contains(cache)) {
            // so we don't need to check the existance in the following code
            toBeExported.emplace(std::make_pair(cache, Selected{}));
        }

        ImGui::SeparatorText("Countries");
        paintCountryInfo(selectAll);

        ImGui::SeparatorText("Cities");
        paintCityInfo(selectAll);

        ImGui::SeparatorText("Note");
        paintNote(selectAll);

        if (auto& selection = toBeExported[cache]; selection.countries.empty() && 
                                                selection.cities.empty() && 
                                                selection.note == false) {
            toBeExported.erase(cache);
        }
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
}

void ExportWidget::paintCountryInfo(bool selectAll)
{
    for (auto& countryInfoWidget : countryInfoWidgets) {

        selectCountry(countryInfoWidget.getName(), selectAll);
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

        selectCity(city.name, selectAll);
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
        selectNote(selectAll);
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

void ExportWidget::processResult()
{
    if (result) {
        if (!exporter) {
            if(auto ret = persistence::ExporterImporterFactory::getInstance().createExporter(exportFormat); ret) {
                exporter = std::move(*ret);
                for (const auto& [info, selected] : toBeExported) {
                    persistence::Data out{info->year};
                    for (const auto& country : info->countries) {
                        if (selected.countries.contains(country.name)) {
                            out.countries.emplace_back(country);
                        }
                    }

                    for (const auto& city : info->cities) {
                        if (selected.cities.contains(city.name)) {
                            out.cities.emplace_back(city);
                        }
                    }

                    if (selected.note) {
                        out.note = info->note;
                    }

                    exporter->insert(std::move(out));
                }
            }
        }

        if (exporter) {
            if (auto ret = exporter->writeToFile(*result, false); !ret) {
                // fail to write to a file
                ImGui::OpenPopup(SAVE_CONFIRM_POPUP_NAME);
                confirmPopupOpen = true;
            } else {
                isComplete = true;
            }

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

void ExportWidget::selectCountry(const std::string& name, bool selectAll)
{
    doSelect(name, toBeExported[cache].countries, selectAll);
}

void ExportWidget::selectCity(const std::string& name, bool selectAll)
{
    doSelect(name, toBeExported[cache].cities, selectAll);
}

void ExportWidget::selectNote(bool selectAll)
{
    bool tick = selectAll || toBeExported[cache].note;

    ImGui::Checkbox("##note", &tick);

    if (tick) {
        toBeExported[cache].note = true;
    } else {
        toBeExported[cache].note = false;
    }

    selectAlls[year] = selectAlls[year] & tick;
}

void ExportWidget::doSelect(const std::string& name, std::set<std::string, CompareString>& container, bool selectAll)
{
    bool tick = selectAll || container.contains(name);

    ImGui::Checkbox(("##" + name).c_str(), &tick);

    if (tick) {
        container.insert(name);
    } else {
        container.erase(name);
    }

    selectAlls[year] = selectAlls[year] & tick;
}
}