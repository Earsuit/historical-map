#include "src/ui/ExportInfoWidget.h"
#include "src/ui/Util.h"
#include "src/logger/LoggerManager.h"
#include "src/presentation/Util.h"
#include "src/util/Signal.h"
#include "src/util/ExecuteablePath.h"

#include "ImFileDialog.h"
#include "external/imgui/imgui.h"

#include <libintl.h>

namespace ui {
#define __(x) x     // gettext translation registration for constexpr

const auto TO_SOURCE = "Export";
constexpr auto SELECT_FORMAT_POPUP_NAME = "Select format";
constexpr auto SAVE_DIALOG_KEY = "ExportDialog";
constexpr auto EXPORT_PROGRESS_POPUP_NAME = __("Exporting");
constexpr auto DONE_BUTTON_LABEL = __("Done");
constexpr auto EXPORT_FAIL_POPUP_NAME = __("Error");
constexpr auto SELECT_MULTIPLE_YEAR_POPUP_NAME = "Select multiple years";
constexpr auto SELECT_MULTI_YEAR_YEAR_CONSTRAINTS = __("Start year must less than end year");
constexpr auto PROCESS_MULTI_YEAR_SELECTION_POPUP_NAME = __("Selecting");
constexpr auto PROGRESS_BAR_SIZE = ImVec2{400, 0};
constexpr auto LOGGER_NAME = "ExportInfoWidget";

ExportInfoWidget::ExportInfoWidget():
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)}, 
    infoPresenter{model::PERMENANT_SOURCE},
    infoSelectorPresenter{model::PERMENANT_SOURCE, TO_SOURCE},
    exportPresenter{TO_SOURCE}
{
    util::signal::connect(&infoPresenter, 
                          &presentation::HistoricalInfoPresenter::setCountriesUpdated,
                          this, 
                          &ExportInfoWidget::setRefreshCountries);
    util::signal::connect(&infoPresenter, 
                          &presentation::HistoricalInfoPresenter::setCityUpdated,
                          this, 
                          &ExportInfoWidget::setRefreshCities);
    util::signal::connect(&infoPresenter, 
                          &presentation::HistoricalInfoPresenter::setNoteUpdated,
                          this, 
                          &ExportInfoWidget::setRefreshNote);
    util::signal::connect(&infoSelectorPresenter, 
                          &presentation::InfoSelectorPresenter::setRefreshSelectAll,
                          this, 
                          &ExportInfoWidget::setRefreshSelectAll);
    util::signal::connect(&yearPresenter, 
                          &presentation::DatabaseYearPresenter::onYearChange,
                          this, 
                          &ExportInfoWidget::setRefreshAll);

    setRefreshAll(yearPresenter.handelGetYear());
}

ExportInfoWidget::~ExportInfoWidget()
{
    util::signal::disconnectAll(&infoPresenter, 
                                &presentation::HistoricalInfoPresenter::setCountriesUpdated,
                                this);
    util::signal::disconnectAll(&infoPresenter, 
                                &presentation::HistoricalInfoPresenter::setCityUpdated,
                                this);
    util::signal::disconnectAll(&infoPresenter, 
                                &presentation::HistoricalInfoPresenter::setNoteUpdated,
                                this);
    util::signal::disconnectAll(&infoSelectorPresenter, 
                                &presentation::InfoSelectorPresenter::setRefreshSelectAll,
                                this);
    util::signal::disconnectAll(&yearPresenter, 
                                &presentation::DatabaseYearPresenter::onYearChange,
                                this);
}

void ExportInfoWidget::setRefreshAll(int year) noexcept
{
    currentYear = year;
    setRefreshCountries();
    setRefreshCities();
    setRefreshNote();
    setRefreshSelectAll();
}

void ExportInfoWidget::displayYearControlSection()
{
    int year = currentYear;
    if (ImGui::SliderInt("##", &year, yearPresenter.handleGetMinYear(), yearPresenter.handleGetMaxYear(), gettext("Year %d"), ImGuiSliderFlags_AlwaysClamp)) {
        yearPresenter.handleSetYear(year);
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
    helpMarker(gettext("Ctrl + click to maually set the year"));
}

void ExportInfoWidget::displayCoordinate(const std::string& uniqueId, const persistence::Coordinate& coord)
{
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);

    auto latitude = coord.latitude;
    auto longitude = coord.longitude;
    ImGui::PushID(uniqueId.c_str());
    textFloatWithLabelOnLeft(gettext("latitude"), latitude);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::SameLine();
    textFloatWithLabelOnLeft(gettext("longitude"), longitude);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::PopID();
}

void ExportInfoWidget::displayCountry(const std::string& name, const std::vector<persistence::Coordinate>& contour)
{
    bool select = selectAll || infoSelectorPresenter.handkeCheckIsCountrySelected(name);
    if (ImGui::Checkbox(("##country" + name).c_str(), &select)) {
        if (select) {
            infoSelectorPresenter.handleSelectCountry(name);
        } else {
            infoSelectorPresenter.handleDeselectCountry(name);
        }
    }
    ImGui::SameLine();

    if (ImGui::TreeNode((name + "##country").c_str())) {
        int idx = 0;
        for (auto& coordinate : contour) {
            displayCoordinate(name + std::to_string(idx++), coordinate);
        }

        ImGui::TreePop();
        ImGui::Spacing();
    }
}

void ExportInfoWidget::displayCity(const std::string& name, const persistence::Coordinate& coord)
{
    bool select = selectAll || infoSelectorPresenter.handleCheckIsCitySelected(name);
    if (ImGui::Checkbox(("##city" + name).c_str(), &select)) {
        if (select) {
            infoSelectorPresenter.handleSelectCity(name);
        } else {
            infoSelectorPresenter.handleDeselectCity(name);
        }
    }
    ImGui::SameLine();

    if (ImGui::TreeNode((name + "##city").c_str())) {
        displayCoordinate(name + "city", coord);
        ImGui::TreePop();
        ImGui::Spacing();
    }
}

void ExportInfoWidget::displayNote()
{
    if (!note.empty()) {
        ImGui::SeparatorText(gettext("Note"));

        bool select = selectAll || infoSelectorPresenter.handleCheckIsNoteSelected();
        if (ImGui::Checkbox("##note", &select)) {
            if (select) {
                infoSelectorPresenter.handleSelectNote();
            } else {
                infoSelectorPresenter.handleDeselectNote();
            }
        }
        ImGui::TextUnformatted(note.c_str(), note.c_str() + note.size());
    }
}

void ExportInfoWidget::paint()
{
    if (ImGui::Begin(INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar)) {
        displayYearControlSection();
        infoPresenter.clearHoveredCoord();

        updateCountryResources();
        updateCityResources();
        updateNoteResources();
        updateSelectAll();

        if (ImGui::Button(gettext("Export as"))) {
            ImGui::OpenPopup(SELECT_FORMAT_POPUP_NAME);
        }
        ImGui::SameLine();
        if (ImGui::Button(gettext("Cancel"))) {
            isComplete = true;
        }

        displayExportPopup();

        if (ImGui::Checkbox(gettext("Select all"), &selectAll)) {
            if (selectAll) {
                infoSelectorPresenter.handleSelectAll();
            } else {
                infoSelectorPresenter.handleDeselectAll();
            }
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(SELECT_MULTIPLE_YEAR_POPUP_NAME);
            startYear = currentYear;
            endYear = currentYear;
        }
        ImGui::SameLine();
        helpMarker(gettext("Right click to select all for multiple years"));

        displaySelectAllForMultipleYearsPopup();

        ImGui::SeparatorText(gettext("Countries"));
        for (const auto& [country, contour] : countries) {
            displayCountry(country, contour);
        }

        ImGui::SeparatorText(gettext("Cities"));
        for (const auto& [city, coord] : cities) {
            displayCity(city, coord);
        }

        displayNote();

        ImGui::End();
    }
}

void ExportInfoWidget::displayExportPopup()
{
    if (ImGui::BeginPopup(SELECT_FORMAT_POPUP_NAME)) {
        for (const auto& format : exportPresenter.handleRequestSupportedFormat()) {
            if(ImGui::Selectable(format.c_str())) {
                if (const auto ret = exportPresenter.handleSetFormat(format); ret) {
                    ImGui::CloseCurrentPopup();
                    ifd::FileDialog::getInstance().save(SAVE_DIALOG_KEY, gettext("Export historical info"), "*." + format + " {." + format +"}", util::getAppBundlePath().parent_path());
                } else {
                    logger.error("Not supported export format {}", format);
                }
            }
        }
        ImGui::EndPopup();
    }

    if (ifd::FileDialog::getInstance().isDone(SAVE_DIALOG_KEY)) {
        if (ifd::FileDialog::getInstance().hasResult()) {
            const std::string file = ifd::FileDialog::getInstance().getResult().u8string();
            exportPresenter.handleDoExport(file);
            ImGui::OpenPopup(EXPORT_PROGRESS_POPUP_NAME);
            exportFailPopup = false;
            openFailPopup = false;
            exportComplete = false;
        } 
        ifd::FileDialog::getInstance().close();
    }
    
    if (ImGui::BeginPopupModal(gettext(EXPORT_PROGRESS_POPUP_NAME), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!exportComplete) {
            if (auto ret = exportPresenter.handleCheckExportComplete(); ret) {
                exportComplete = ret.value();
            } else {
                ImGui::CloseCurrentPopup();
                openFailPopup = true;
                exportComplete = true;
                errorMsg = ret.error().msg;
            }
        }

        if (!openFailPopup) {
            simpleProgressDisplayer(exportPresenter.handleRequestExportProgress(),
                                    gettext(DONE_BUTTON_LABEL),
                                    exportComplete,
                                    [this](){
                                        this->isComplete = true;
                                    });
        }

        ImGui::EndPopup();
    }

    if (openFailPopup) {
        ImGui::OpenPopup(EXPORT_FAIL_POPUP_NAME);
        exportFailPopup = true;
        openFailPopup = false;
    }

    if (ImGui::BeginPopupModal(gettext(EXPORT_FAIL_POPUP_NAME), &exportFailPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(gettext("Failed to export: %s"), errorMsg.c_str());
        ImGui::EndPopup();
    }
}

void ExportInfoWidget::displaySelectAllForMultipleYearsPopup()
{
    if (ImGui::BeginPopup(SELECT_MULTIPLE_YEAR_POPUP_NAME)) {
        ImGui::InputInt(gettext("Start"), &startYear);
        ImGui::InputInt(gettext("End"), &endYear);
        helpMarker(gettext(SELECT_MULTI_YEAR_YEAR_CONSTRAINTS));

        if (ImGui::Button(gettext("Select"))) {
            if (startYear < endYear) {
                infoSelectorPresenter.handleSelectAllForMultipleYears(startYear, endYear);
                processMultiYearSelection = true;
                processMultiYearSelectionComplete = false;
                ImGui::CloseCurrentPopup();
            } else {
                logger.error(SELECT_MULTI_YEAR_YEAR_CONSTRAINTS);
            }
        }

        ImGui::EndPopup();

        if (processMultiYearSelection) {
            ImGui::OpenPopup(PROCESS_MULTI_YEAR_SELECTION_POPUP_NAME);
        }
    }

    if (ImGui::BeginPopupModal(gettext(PROCESS_MULTI_YEAR_SELECTION_POPUP_NAME), &processMultiYearSelection, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (!processMultiYearSelectionComplete) {
            processMultiYearSelectionComplete = infoSelectorPresenter.handleCheckSelectAllForMultipleYearsComplete();
        }
        
        simpleProgressDisplayer(infoSelectorPresenter.handleGetSelectAllForMultipleYearsProgress(),
                                gettext(DONE_BUTTON_LABEL),
                                processMultiYearSelectionComplete,
                                [](){});

        ImGui::EndPopup();
    }

    // the user stops the process manually
    if (!processMultiYearSelection && !processMultiYearSelectionComplete) {
        infoSelectorPresenter.handleCancelSelectAllForMultipleYears();
    }
}

void ExportInfoWidget::updateCountryResources()
{
    if (countryResourceUpdated) {
        countryResourceUpdated = false;
        countries.clear();
        for (const auto& country : infoPresenter.handleRequestCountryList()) {
            countries.emplace(std::make_pair(country, std::vector<persistence::Coordinate>{}));
            for (const auto& coord : infoPresenter.handleRequestContour(country)) {
                countries[country].emplace_back(coord);
            }
        }
    }
}

void ExportInfoWidget::updateCityResources()
{
    if (cityResourceUpdated) {
        cityResourceUpdated = false;
        cities.clear();
        for (const auto& city : infoPresenter.handleRequestCityList()) {
            if (const auto coord =infoPresenter.handleRequestCityCoordinate(city); coord) {
                cities.emplace(std::make_pair(city, *coord));
            }
        }
    }
}

void ExportInfoWidget::updateNoteResources()
{
    if (noteResourceUpdated) {
        noteResourceUpdated = false;
        note = infoPresenter.handleGetNote();
    }
}

void ExportInfoWidget::updateSelectAll()
{
    if (needUpdateSelectAll) {
        needUpdateSelectAll = false;
        selectAll = infoSelectorPresenter.handleCheckIsAllSelected();
    }
}
}