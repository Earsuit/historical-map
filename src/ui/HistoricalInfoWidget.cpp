#include "src/ui/HistoricalInfoWidget.h"
#include "src/ui/Util.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

namespace ui {
constexpr int SLIDER_WIDTH = 40;
constexpr int NAME_INPUT_WIDTH = 100;
constexpr auto POPUP_WINDOW_NAME = "Save for years";
constexpr auto PROGRESS_POPUP_WINDOW_NAME = "Saving";
constexpr float STEP = 0;
constexpr float STEP_FAST = 0;
constexpr auto DECIMAL_PRECISION = "%.2f";

void HistoricalInfoWidget::historyInfo(int year)
{
    if (currentYear != year) {
        currentYear = year;
        countryNewCoordinate.clear();
    }

    if (ImGui::Button("Refresh")) {
        logger->debug("Refresh data of year {} from database.", year);

        databaseAccessPresenter.handleRefresh();        
    }
    ImGui::SameLine();

    if (ImGui::Button("Save")) {
        databaseAccessPresenter.handleSave(year, year);
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        startYear = endYear = year;
        ImGui::OpenPopup(POPUP_WINDOW_NAME);
    }
    savePopupWindow();
    saveProgressPopUp();

    infoPresenter.clearHoveredCoord();

    ImGui::SameLine();
    helpMarker("Right click \"Save\" button to save for several years.");

    ImGui::SeparatorText("Countries");
    displayCountryInfo();

    ImGui::SeparatorText("Cities");
    displayCityInfo();

    ImGui::SeparatorText("Note");
    displayNote();
}

void HistoricalInfoWidget::displayCountryInfo()
{
    infoPresenter.handleDisplayCountries();
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

void HistoricalInfoWidget::displayCountry(const std::string& name)
{
    if (ImGui::TreeNode((name + "##country").c_str())) {
        infoPresenter.handleDisplayCountry(name);

        if (!countryNewCoordinate.contains(name)) {
            countryNewCoordinate.emplace(std::make_pair(name, std::make_pair(std::string{}, std::string{})));
        }

        auto& [latitude, longitude] = countryNewCoordinate[name];

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

persistence::Coordinate HistoricalInfoWidget::displayCoordinate(const persistence::Coordinate& coord)
{
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);

    auto latitude = coord.latitude;
    auto longitude = coord.longitude;
    ImGui::InputFloat("latitude", &latitude, STEP, STEP_FAST, DECIMAL_PRECISION);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::SameLine();
    ImGui::InputFloat("longitude", &longitude, STEP, STEP_FAST, DECIMAL_PRECISION);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }

    return persistence::Coordinate{latitude, longitude};
}

void HistoricalInfoWidget::displayCityInfo()
{
    infoPresenter.handleDisplayCities();

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

void HistoricalInfoWidget::displayCity(const std::string& name)
{
    if (ImGui::TreeNode((name + "##city").c_str())) {
        infoPresenter.handleDisplayCity(name);

        ImGui::TreePop();
        ImGui::Spacing();
    }

    ImGui::SameLine();
    if (ImGui::Button("Remove")) {
        this->logger->debug("Delete city {}", name);
        infoPresenter.handleRemoveCity(name);
    }
}

void HistoricalInfoWidget::displayNote()
{
    if (auto note = infoPresenter.handleGetNote(); note) {
        if (ImGui::Button("Clear")) {
            infoPresenter.handleUpdateNote("");
        }

        if (ImGui::InputTextMultiline("##note", &(*note), ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_AllowTabInput)) {
            infoPresenter.handleUpdateNote(*note);
        }
    }
}

void HistoricalInfoWidget::savePopupWindow()
{
    if (ImGui::BeginPopup(POPUP_WINDOW_NAME)) {
        ImGui::SeparatorText(POPUP_WINDOW_NAME);
        ImGui::InputInt("Start", &startYear);
        ImGui::InputInt("End", &endYear);
        
        if (ImGui::Button("Save##ForYears")) {
            if (databaseAccessPresenter.handleSave(startYear, endYear)) {
                ImGui::CloseCurrentPopup();
                ImGui::OpenPopup(PROGRESS_POPUP_WINDOW_NAME);
            }
        }
        
        ImGui::EndPopup();
    }
}

void HistoricalInfoWidget::saveProgressPopUp()
{
    if (ImGui::BeginPopupModal(PROGRESS_POPUP_WINDOW_NAME)) {
        simpleProgressDisplayer(databaseAccessPresenter.getProgress(),
                                "Done",
                                databaseAccessPresenter.isSaveComplete(),
                                [](){});
    }
}
}