#include "src/ui/DefaultInfoWidget.h"
#include "src/ui/Util.h"
#include "src/util/Signal.h"
#include "src/logger/LoggerManager.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

#include <libintl.h>

namespace ui {
constexpr int NAME_INPUT_WIDTH = 100;
constexpr auto POPUP_WINDOW_NAME = "Save to years";
constexpr auto PROGRESS_POPUP_WINDOW_NAME = "Saving";
constexpr auto LOGGER_NAME = "DefaultInfoWidget";

DefaultInfoWidget::DefaultInfoWidget(): 
    logger{logger::LoggerManager::getInstance().getLogger(LOGGER_NAME)}, 
    infoPresenter{model::PERMENANT_SOURCE},
    databaseSaverPresenter{model::PERMENANT_SOURCE}
{
    util::signal::connect(&infoPresenter, 
                          &presentation::HistoricalInfoPresenter::setCountriesUpdated,
                          this, 
                          &DefaultInfoWidget::setRefreshCountries);
    util::signal::connect(&infoPresenter, 
                          &presentation::HistoricalInfoPresenter::setCityUpdated,
                          this, 
                          &DefaultInfoWidget::setRefreshCities);
    util::signal::connect(&infoPresenter, 
                          &presentation::HistoricalInfoPresenter::setNoteUpdated,
                          this, 
                          &DefaultInfoWidget::setRefreshNote);
    util::signal::connect(&infoPresenter, 
                          &presentation::HistoricalInfoPresenter::setModificationState,
                          this, 
                          &DefaultInfoWidget::setUnsavedState);
    util::signal::connect(&yearPresenter, 
                          &presentation::DatabaseYearPresenter::onYearChange,
                          this, 
                          &DefaultInfoWidget::setOnYearChange);

    currentYear = yearPresenter.handelGetYear();
}

DefaultInfoWidget::~DefaultInfoWidget()
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
    util::signal::disconnectAll(&infoPresenter, 
                                &presentation::HistoricalInfoPresenter::setModificationState,
                                this);
    util::signal::disconnectAll(&yearPresenter, 
                                &presentation::DatabaseYearPresenter::onYearChange,
                                this);
}

void DefaultInfoWidget::displayYearControlSection()
{
    std::string yearLabel = gettext("Year %d");
    if (isUnsaved) {
        yearLabel += " *";
    }

    int year = currentYear;
    if (ImGui::SliderInt("##", &year, yearPresenter.handleGetMinYear(), yearPresenter.handleGetMaxYear(), yearLabel.c_str(), ImGuiSliderFlags_AlwaysClamp)) {
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

void DefaultInfoWidget::setOnYearChange(int year) noexcept
{
    currentYear = year;
    clearNewInfoEntry = true;
    isUnsaved = infoPresenter.handleCheckIsModified();
    setRefreshCountries();
    setRefreshCities();
    setRefreshNote();
}

void DefaultInfoWidget::paint()
{
    if (ImGui::Begin(INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar)) {
        displayYearControlSection();

        if (ImGui::Button(gettext("Refresh"))) {
            logger.debug("Refresh data of year {} from database.", currentYear);
            
            infoPresenter.handleClearHistoricalInfo();
            yearPresenter.handleSetYear(currentYear);
        }
        ImGui::SameLine();

        updateCountryResources();
        updateCityResources();
        updateNoteResources();
        updateNewInfoEntry();

        if (ImGui::Button(gettext("Save current year"))) {
            databaseSaverPresenter.handleSaveSameForRange(currentYear, currentYear);
            ImGui::OpenPopup(PROGRESS_POPUP_WINDOW_NAME);
        }

        ImGui::SameLine();
        if (ImGui::Button(gettext("Save to years"))) {
            startYear = endYear = currentYear;
            ImGui::OpenPopup(POPUP_WINDOW_NAME);
        }

        savePopupWindow();
        saveProgressPopUp();

        infoPresenter.clearHoveredCoord();

        ImGui::SeparatorText(gettext("Countries"));
        displayCountryInfos();

        ImGui::SeparatorText(gettext("Cities"));
        displayCityInfos();

        ImGui::SeparatorText(gettext("Note"));
        displayNote();

        ImGui::End();
    }
}

void DefaultInfoWidget::displayCountryInfos()
{
    for (const auto& [country, contour] : countries) {
        displayCountry(country, contour);
    }

    ImGui::PushItemWidth(NAME_INPUT_WIDTH);
    ImGui::InputTextWithHint("##Country name", gettext("Country name"), &countryName);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(gettext("Add country")) && !countryName.empty()) {
        logger.debug("Add country button pressed to add country {} at year {}", countryName, currentYear);
        infoPresenter.handleAddCountry(countryName);
        countryName.clear();
    }
}

void DefaultInfoWidget::displayCountry(const std::string& name, const std::vector<persistence::Coordinate>& contour)
{
    if (ImGui::TreeNode((name + "##country").c_str())) {
        int idx = 0;
        for (const auto& coordinate : contour) {
            const auto newCoordinate = displayCoordinate(name + std::to_string(idx), coordinate);

            if (newCoordinate != coordinate) {
                infoPresenter.handleUpdateContour(name, idx, newCoordinate);
            }

            ImGui::SameLine();
            if (ImGui::Button((gettext("Delete##") + std::to_string(idx) + name).c_str())) {
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
        ImGui::InputText(gettext("latitude"), &latitude);
        ImGui::SameLine();
        ImGui::InputText(gettext("longitude"), &longitude);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button(gettext("Add")) && !latitude.empty() && !longitude.empty()) {
            float lat, lon;
            try {
                lat = std::stod(latitude);
                lon = std::stod(longitude);
            }
            catch (const std::exception &exc) {
                logger.error("Invalid value for new coordinate for country {}.", name);
                return;
            }

            logger.debug("Add coordinate lat={}, lon={}.", latitude, longitude);
            infoPresenter.handleExtendContour(name, persistence::Coordinate{lat, lon});
            latitude.clear();
            longitude.clear();
        }

        if (ImGui::Button(gettext("Delete country"))) {
            this->logger.debug("Delete country {}", name);
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
    inputFloatWithLabelOnLeft(gettext("latitude"), latitude);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::SameLine();
    inputFloatWithLabelOnLeft(gettext("longitude"), longitude);
    if (ImGui::IsItemHovered()) {
        infoPresenter.setHoveredCoord(coord);
    }
    ImGui::PopID();

    return persistence::Coordinate{latitude, longitude};
}

void DefaultInfoWidget::displayCityInfos()
{
    for (const auto& [city, coord] : cities) {
        displayCity(city, coord);
    }

    ImGui::PushItemWidth(NAME_INPUT_WIDTH);
    ImGui::InputTextWithHint("##City name", gettext("City name"), &newCityName);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);
    ImGui::InputTextWithHint("##CityLatitude" , gettext("lat"), &newCityLatitude);
    ImGui::SameLine();
    ImGui::InputTextWithHint("##CityLongitude", gettext("lon"), &newCityLongitude);
    ImGui::PopItemWidth();
    if (ImGui::Button(gettext("Add city")) && !newCityName.empty() && !newCityLongitude.empty() && !newCityLatitude.empty()) {
        float lat, lon;
        try {
            lat = std::stod(newCityLatitude);
            lon = std::stod(newCityLongitude);
        }
        catch (const std::exception &exc) {
            logger.error("Invalid value of new coordinate for city {}.", newCityName);
            return;
        }

        infoPresenter.handleAddCity(newCityName, persistence::Coordinate{lat, lon});
        logger.debug("Add city {}", newCityName);
        newCityName.clear();
        newCityLatitude.clear();
        newCityLongitude.clear();
    }
}

void DefaultInfoWidget::displayCity(const std::string& name, const persistence::Coordinate& coord)
{
    if (ImGui::TreeNode((name + "##city").c_str())) {
        const auto newCoordinate = displayCoordinate(name + "city", coord);

        if (newCoordinate != coord) {
            infoPresenter.handleUpdateCityCoordinate(name, newCoordinate);
        }

        if (ImGui::Button(gettext("Remove"))) {
            this->logger.debug("Delete city {}", name);
            infoPresenter.handleRemoveCity(name);
        }

        ImGui::TreePop();
        ImGui::Spacing();
    }
}

void DefaultInfoWidget::displayNote()
{
    if (ImGui::Button(gettext("Clear"))) {
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
        ImGui::SeparatorText(gettext(POPUP_WINDOW_NAME));
        ImGui::InputInt(gettext("Start"), &startYear);
        ImGui::InputInt(gettext("End"), &endYear);
        
        if (ImGui::Button(gettext("Save"))) {
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
                                gettext("Done"),
                                databaseSaverPresenter.isSaveComplete(),
                                [](){});
        ImGui::EndPopup();
    }
}

void DefaultInfoWidget::updateCountryResources()
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

void DefaultInfoWidget::updateCityResources()
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

void DefaultInfoWidget::updateNoteResources()
{
    if (noteResourceUpdated) {
        noteResourceUpdated = false;
        note = infoPresenter.handleGetNote();
    }
}

void DefaultInfoWidget::updateNewInfoEntry()
{
    if (clearNewInfoEntry) {
        clearNewInfoEntry = false;
        countryNewCoordinateCache.clear();
        newCityName.clear();
        newCityLatitude.clear();
        newCityLongitude.clear();
    }
}
}