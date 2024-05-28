#include "src/ui/ImportInfoWidget.h"
#include "src/ui/Util.h"
#include "src/presentation/Util.h"
#include "src/logger/Util.h"

#include "imgui.h"
#include "ImFileDialog.h"

namespace ui {
const auto SELECTION = "Selected";
constexpr auto FILE_SELECT_POPUP_NAME = "Select file";
constexpr auto IMPORT_PROGRESS_POPUP_NAME = "Loading";
constexpr auto IMPORT_COMPLETE_BUTTON = "Complete";
constexpr auto IMPORT_FAIL_POPUP_NAME = "Import fail";
constexpr auto DONE_BUTTON = "Done";
constexpr auto WRITE_TO_DATABASE_PROGRESS_POPUP = "Write to database";

ImportInfoWidget::ImportInfoWidget():
    logger{spdlog::get(logger::LOGGER_NAME)}, 
    databaseInfoPresenter{presentation::DEFAULT_HISTORICAL_INFO_SOURCE},
    importInfoPresenter{presentation::IMPORT_SOURCE},
    infoSelectorPresenter{presentation::IMPORT_SOURCE, SELECTION},
    yearPresenter{presentation::IMPORT_SOURCE},
    importPresenter{presentation::IMPORT_SOURCE},
    databaseSaverPresenter{SELECTION}
{
    ifd::FileDialog::getInstance().open(FILE_SELECT_POPUP_NAME, FILE_SELECT_POPUP_NAME, fileExtensionFormat());
}

void ImportInfoWidget::displayYearControlSection()
{
    currentYear = yearPresenter.handelGetYear();
    if (ImGui::SliderInt("##", &currentYear, yearPresenter.handleGetMinYear(), yearPresenter.handleGetMaxYear(), "Year %d", ImGuiSliderFlags_AlwaysClamp)) {
        yearPresenter.handleSetYear(currentYear);
    }
    
    ImGui::SameLine();
    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        yearPresenter.handleMoveYearBackward();
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
        yearPresenter.handleMoveYearForward();
    }
    ImGui::PopButtonRepeat();

    ImGui::SameLine();
    helpMarker("Ctrl + click to maually set the year");

    currentYear = yearPresenter.handelGetYear();
}

std::string ImportInfoWidget::fileExtensionFormat() const
{
    const auto formats = importPresenter.handleGetSupportedFormat();
    std::string extensions = "(";
    for (const auto& format: formats) {
        extensions += "*." + format + ";";
    }
    extensions.back() = ')';
    extensions += "{";
    for (const auto& format: formats) {
        extensions += "." + format + ",";
    }
    extensions.back() = '}';

    return extensions;
}

void ImportInfoWidget::doImport()
{
    if (ifd::FileDialog::getInstance().isDone(FILE_SELECT_POPUP_NAME)) {
        if (ifd::FileDialog::getInstance().hasResult()) {
            const std::string file = ifd::FileDialog::getInstance().getResult().u8string();
            logger->debug("Open file {}", file);
            importPresenter.handleDoImport(file);
            openErrorPopup = false;
            ImGui::OpenPopup(IMPORT_PROGRESS_POPUP_NAME);
        } else {
            isComplete = true;
        }
        ifd::FileDialog::getInstance().close();
    }

    if (ImGui::BeginPopupModal(IMPORT_PROGRESS_POPUP_NAME, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Imported: ");
        ImGui::SameLine();

        if (ImGui::BeginListBox("##")) {
            for (const auto year : importPresenter.handleGetImportedYears()) {
                ImGui::Text("Year %d", year);
            }
            
            ImGui::EndListBox();
        }

        if (!importComplete) {
            if (auto ret = importPresenter.handleCheckImportComplete(); ret) {
                if (importComplete = ret.value(); importComplete) {
                    yearPresenter.initYearsList();
                }
            } else {
                ImGui::CloseCurrentPopup();
                errorMsg = ret.error().msg;
                openErrorPopup = true;
            }
        }

        centeredEnableableButton(IMPORT_COMPLETE_BUTTON,
                                 this->importComplete,
                                 [this](){
                                    // start from the first year
                                    this->yearPresenter.handleSetYear(this->yearPresenter.handleGetMinYear());
                                 });

        ImGui::EndPopup();

        if (openErrorPopup) {
            ImGui::OpenPopup(IMPORT_FAIL_POPUP_NAME);
        }
    }

    if (ImGui::BeginPopupModal(IMPORT_FAIL_POPUP_NAME, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        alignForWidth(ImGui::CalcTextSize(errorMsg.c_str()).x);
        ImGui::Text("%s", errorMsg.c_str());

        alignForWidth(ImGui::CalcTextSize(DONE_BUTTON).x);
        if (ImGui::Button(DONE_BUTTON)) {
            ImGui::CloseCurrentPopup();
            isComplete = true;
        }

        ImGui::EndPopup();
    }
}

void ImportInfoWidget::paint()
{
    doImport();

    if (ImGui::Begin(INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar)) {
        if (importComplete) {
            displayYearControlSection();

            if (ImGui::Button("Confirm")) {
                databaseSaverPresenter.handleSaveAll();
                ImGui::OpenPopup(WRITE_TO_DATABASE_PROGRESS_POPUP);
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                isComplete = true;
            }

            displaySaveToDatabasePopup();

            selectAll = infoSelectorPresenter.handleCheckIsAllSelected();
            if (ImGui::Checkbox("Select all", &selectAll)) {
                if (selectAll) {
                    infoSelectorPresenter.handleSelectAll();
                } else {
                    infoSelectorPresenter.handleDeselectAll();
                }
            }

            if (ImGui::TreeNode("Imported")) {
                ImGui::SeparatorText("Countries");
                for (const auto& country : importInfoPresenter.handleRequestCountryList()) {
                    selectCountry(country);
                    ImGui::SameLine();
                    displayCountry(importInfoPresenter, country);
                }

                ImGui::SeparatorText("Cities");
                for (const auto& city : importInfoPresenter.handleRequestCityList()) {
                    selectCity(city);
                    ImGui::SameLine();
                    displayCity(importInfoPresenter, city);
                }

                ImGui::SeparatorText("Note");
                selectNote(importInfoPresenter);
                displayNote(importInfoPresenter);

                ImGui::TreePop();
                ImGui::Spacing();
            }

            if (ImGui::TreeNode("Database")) {
                ImGui::SeparatorText("Countries");
                for (const auto& country : databaseInfoPresenter.handleRequestCountryList()) {
                    displayCountry(databaseInfoPresenter, country);
                }

                ImGui::SeparatorText("Cities");
                for (const auto& city : databaseInfoPresenter.handleRequestCityList()) {
                    displayCity(databaseInfoPresenter, city);
                }

                ImGui::SeparatorText("Note");
                displayNote(databaseInfoPresenter);

                ImGui::TreePop();
                ImGui::Spacing();
            }
        }

        ImGui::End();
    }
}

void ImportInfoWidget::selectCountry(const std::string& name)
{
    bool select = selectAll || infoSelectorPresenter.handkeCheckIsCountrySelected(name);
    if (ImGui::Checkbox(("##country" + name).c_str(), &select)) {
        if (select) {
            infoSelectorPresenter.handleSelectCountry(name);
        } else {
            infoSelectorPresenter.handleDeselectCountry(name);
        }
    }
}

void ImportInfoWidget::displayCountry(presentation::HistoricalInfoPresenter& infoPresenter, const std::string& name)
{
    if (ImGui::TreeNode((name + "##country").c_str())) {
        int idx = 0;
        for (auto& coordinate : infoPresenter.handleRequestContour(name)) {
            displayCoordinate(infoPresenter, name + std::to_string(idx++), coordinate);
        }

        ImGui::TreePop();
        ImGui::Spacing();
    }
}

void ImportInfoWidget::selectCity(const std::string& name)
{
    bool select = selectAll || infoSelectorPresenter.handleCheckIsCitySelected(name);
    if (ImGui::Checkbox(("##city" + name).c_str(), &select)) {
        if (select) {
            infoSelectorPresenter.handleSelectCity(name);
        } else {
            infoSelectorPresenter.handleDeselectCity(name);
        }
    }
}

void ImportInfoWidget::displayCity(presentation::HistoricalInfoPresenter& infoPresenter, const std::string& name)
{
    if (const auto ret = infoPresenter.handleRequestCityCoordinate(name); ret) {
        if (ImGui::TreeNode((name + "##city").c_str())) {
            displayCoordinate(infoPresenter, name + "city", *ret);
            ImGui::TreePop();
            ImGui::Spacing();
        }
    }
}

void ImportInfoWidget::selectNote(const presentation::HistoricalInfoPresenter& infoPresenter)
{
    if (auto note = infoPresenter.handleGetNote(); !note.empty()) {
        bool select = selectAll || infoSelectorPresenter.handleCheckIsNoteSelected();
        if (ImGui::Checkbox("##note", &select)) {
            if (select) {
                infoSelectorPresenter.handleSelectNote();
            } else {
                infoSelectorPresenter.handleDeselectNote();
            }
        }
    }
}

void ImportInfoWidget::displayNote(const presentation::HistoricalInfoPresenter& infoPresenter)
{
    if (auto note = infoPresenter.handleGetNote(); !note.empty()) {
        ImGui::TextUnformatted(note.c_str(), note.c_str() + note.size());
    }
}

void ImportInfoWidget::displayCoordinate(presentation::HistoricalInfoPresenter& infoPresenter, 
                                         const std::string& uniqueId, 
                                         const persistence::Coordinate& coord)
{
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);

    auto latitude = coord.latitude;
    auto longitude = coord.longitude;
    ImGui::PushID(uniqueId.c_str());
    textFloatWithLabelOnLeft("latitude", latitude);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::SameLine();
    textFloatWithLabelOnLeft("longitude", longitude);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::PopID();
}

void ImportInfoWidget::displaySaveToDatabasePopup()
{
    if (ImGui::BeginPopupModal(WRITE_TO_DATABASE_PROGRESS_POPUP)) {
        simpleProgressDisplayer(databaseSaverPresenter.getProgress(),
                                DONE_BUTTON,
                                databaseSaverPresenter.isSaveComplete(),
                                [this](){
                                    this->isComplete = true;
                                });
        ImGui::EndPopup();
    }
}
}