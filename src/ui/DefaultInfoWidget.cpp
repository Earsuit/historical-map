#include "src/ui/DefaultInfoWidget.h"
#include "src/ui/Util.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

namespace ui {
constexpr int NAME_INPUT_WIDTH = 100;
constexpr auto POPUP_WINDOW_NAME = "Save for years";
constexpr auto PROGRESS_POPUP_WINDOW_NAME = "Saving";
constexpr float STEP = 0;
constexpr float STEP_FAST = 0;
constexpr auto DECIMAL_PRECISION = "%.2f";

void DefaultInfoWidget::displayYearControlSection()
{
    currentYear = yearPresenter.handelGetYear();

    if (ImGui::SliderInt("##", &currentYear, yearPresenter.handleGetMinYear(), yearPresenter.handleGetMaxYear(), "Year %d", ImGuiSliderFlags_AlwaysClamp)) {
        yearPresenter.handleSetYear(currentYear);
        countryNewCoordinateCache.clear();
    }
    
    ImGui::SameLine();
    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        yearPresenter.handleMoveYearBackward();
        countryNewCoordinateCache.clear();
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
        yearPresenter.handleMoveYearForward();
        countryNewCoordinateCache.clear();
    }
    ImGui::PopButtonRepeat();

    ImGui::SameLine();
    helpMarker("Ctrl + click to maually set the year");

    currentYear = yearPresenter.handelGetYear();
}

void DefaultInfoWidget::paint()
{
    if (ImGui::Begin(INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar)) {
        displayYearControlSection();

        if (ImGui::Button("Refresh")) {
            logger->debug("Refresh data of year {} from database.", currentYear);
            
            infoPresenter.handleClearHistoricalInfo();
            yearPresenter.handleSetYear(currentYear);
        }
        ImGui::SameLine();

        if (ImGui::Button("Save")) {
            databaseSaverPresenter.handleSaveSameForRange(currentYear, currentYear);
            ImGui::OpenPopup(PROGRESS_POPUP_WINDOW_NAME);
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            startYear = endYear = currentYear;
            ImGui::OpenPopup(POPUP_WINDOW_NAME);
        }
        savePopupWindow();
        saveProgressPopUp();

        infoPresenter.clearHoveredCoord();

        ImGui::SameLine();
        helpMarker("Right click \"Save\" button to save for several years.");

        ImGui::SeparatorText("Countries");
        displayCountryInfos();

        ImGui::SeparatorText("Cities");
        displayCityInfos();

        ImGui::SeparatorText("Note");
        displayNote();

        ImGui::End();
    }
}

void DefaultInfoWidget::displayCountryInfos()
{
    for (const auto& country : infoPresenter.handleRequestCountryList()) {
        displayCountry(country);
    }

    ImGui::PushItemWidth(NAME_INPUT_WIDTH);
    ImGui::InputTextWithHint("##Country name", "Country name", &countryName);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Add country") && !countryName.empty()) {
        logger->debug("Add country button pressed to add country {} at year {}", countryName, currentYear);
        infoPresenter.handleAddCountry(countryName);
        countryName.clear();
    }
}

void DefaultInfoWidget::displayCountry(const std::string& name)
{
    if (ImGui::TreeNode((name + "##country").c_str())) {
        int idx = 0;
        for (auto& coordinate : infoPresenter.handleRequestContour(name)) {
            const auto newCoordinate = displayCoordinate(name + std::to_string(idx), coordinate);

            if (newCoordinate != coordinate) {
                infoPresenter.handleUpdateContour(name, idx, newCoordinate);
            }

            ImGui::SameLine();
            if (ImGui::Button(("Delete##" + std::to_string(idx) + name).c_str())) {
                infoPresenter.handleDeleteFromContour(name, idx);
            }
            idx++;
        }

        if (!countryNewCoordinateCache.contains(name)) {
            countryNewCoordinateCache.emplace(std::make_pair(name, std::make_pair(std::string{}, std::string{})));
        }

        auto& [latitude, longitude] = countryNewCoordinateCache[name];

        ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);
        // input filed for new coordinate
        ImGui::InputText("Latitude", &latitude);
        ImGui::SameLine();
        ImGui::InputText("Longitude", &longitude);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button("Add") && !latitude.empty() && !longitude.empty()) {
            float lat, lon;
            try {
                lat = std::stod(latitude);
                lon = std::stod(longitude);
            }
            catch (const std::exception &exc) {
                logger->error("Invalid value for new coordinate for country {}.", name);
                return;
            }

            logger->debug("Add coordinate lat={}, lon={}.", latitude, longitude);
            infoPresenter.handleExtendContour(name, persistence::Coordinate{lat, lon});
            latitude.clear();
            longitude.clear();
        }

        if (ImGui::Button("Delete country")) {
            this->logger->debug("Delete country {}", name);
            infoPresenter.handleRemoveCountry(name);
        }

        ImGui::TreePop();
        ImGui::Spacing();
    }
}

persistence::Coordinate DefaultInfoWidget::displayCoordinate(const std::string& uniqueId,
                                                             const persistence::Coordinate& coord)
{
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);

    auto latitude = coord.latitude;
    auto longitude = coord.longitude;
    ImGui::PushID(uniqueId.c_str());
    ImGui::InputFloat("latitude", &latitude, STEP, STEP_FAST, DECIMAL_PRECISION);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::SameLine();
    ImGui::InputFloat("longitude", &longitude, STEP, STEP_FAST, DECIMAL_PRECISION);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::PopID();

    return persistence::Coordinate{latitude, longitude};
}

void DefaultInfoWidget::displayCityInfos()
{
    for (const auto& city : infoPresenter.handleRequestCityList()) {
        displayCity(city);
    }

    ImGui::PushItemWidth(NAME_INPUT_WIDTH);
    ImGui::InputTextWithHint("##City name", "City name", &newCityName);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);
    ImGui::InputTextWithHint("##CityLatitude" ,"Lat", &newCityLatitude);
    ImGui::SameLine();
    ImGui::InputTextWithHint("##CityLongitude", "Lon", &newCityLongitude);
    ImGui::PopItemWidth();
    if (ImGui::Button("Add city") && !newCityName.empty() && !newCityLongitude.empty() && !newCityLatitude.empty()) {
        float lat, lon;
        try {
            lat = std::stod(newCityLatitude);
            lon = std::stod(newCityLongitude);
        }
        catch (const std::exception &exc) {
            logger->error("Invalid value of new coordinate for city {}.", newCityName);
            return;
        }

        infoPresenter.handleAddCity(newCityName, persistence::Coordinate{lat, lon});
        logger->debug("Add city {}", newCityName);
        newCityName.clear();
        newCityLatitude.clear();
        newCityLongitude.clear();
    }
}

void DefaultInfoWidget::displayCity(const std::string& name)
{
    if (const auto ret = infoPresenter.handleRequestCityCoordinate(name); ret) {
        if (ImGui::TreeNode((name + "##city").c_str())) {
            const auto newCoordinate = displayCoordinate(name + "city", *ret);

            if (newCoordinate != *ret) {
                infoPresenter.handleUpdateCityCoordinate(name, newCoordinate);
            }

            if (ImGui::Button("Remove")) {
                this->logger->debug("Delete city {}", name);
                infoPresenter.handleRemoveCity(name);
            }

            ImGui::TreePop();
            ImGui::Spacing();
        }
    }
}

void DefaultInfoWidget::displayNote()
{
    auto note = infoPresenter.handleGetNote();
    if (ImGui::Button("Clear")) {
        infoPresenter.handleUpdateNote("");
    }

    if (ImGui::InputTextMultiline("##note", &note, ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_AllowTabInput)) {
        infoPresenter.handleUpdateNote(note);
    }
}

void DefaultInfoWidget::savePopupWindow()
{
    bool openSavePopup = false;
    if (ImGui::BeginPopup(POPUP_WINDOW_NAME)) {
        ImGui::SeparatorText(POPUP_WINDOW_NAME);
        ImGui::InputInt("Start", &startYear);
        ImGui::InputInt("End", &endYear);
        
        if (ImGui::Button("Save##ForYears")) {
            if (databaseSaverPresenter.handleSaveSameForRange(startYear, endYear)) {
                ImGui::CloseCurrentPopup();
                openSavePopup = true;
            }
        }
        
        ImGui::EndPopup();
    }

    if (openSavePopup) {
        openSavePopup = false;
        ImGui::OpenPopup(PROGRESS_POPUP_WINDOW_NAME);
    }
}

void DefaultInfoWidget::saveProgressPopUp()
{
    if (ImGui::BeginPopupModal(PROGRESS_POPUP_WINDOW_NAME)) {
        simpleProgressDisplayer(databaseSaverPresenter.getProgress(),
                                "Done",
                                databaseSaverPresenter.isSaveComplete(),
                                [](){});
        ImGui::EndPopup();
    }
}
}