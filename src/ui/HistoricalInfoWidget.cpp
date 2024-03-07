#include "src/ui/HistoricalInfoWidget.h"
#include "src/ui/Util.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

namespace ui {
constexpr int MIN_YEAR = -3000;
constexpr int MAX_YEAR = 1911;
constexpr int SLIDER_WIDTH = 40;
constexpr int COORDINATE_INPUT_WIDTH = 50;
constexpr int NAME_INPUT_WIDTH = 100;
constexpr double STEP = 0;
constexpr double STEP_FAST = 0;
constexpr auto POPUP_WINDOW_NAME = "Save for years";

void HistoricalInfoWidget::paint()
{
    ImGui::Begin(HISTORICAL_INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar);

    // there is no year 0
    ImGui::SliderInt("##", &year, MIN_YEAR, MAX_YEAR, "Year %d", ImGuiSliderFlags_AlwaysClamp);
    if (year == 0) {
        year = 1;
    }
    ImGui::SameLine();
    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        year--;
        if (year == 0) {
            year = -1;
        }
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
        year++;
        if (year == 0) {
            year = 1;
        }
    }
    ImGui::PopButtonRepeat();

    ImGui::SameLine();
    helpMarker("Ctrl + click to maually set the year");

    if (database.getWorkLoad() != 0) {
        displaySaveProgress();
    }

    historyInfo();

    ImGui::End();
}

void HistoricalInfoWidget::historyInfo()
{
    selected = std::nullopt;

    if (ImGui::Button("Refresh") || !cache || cache->year != year) {
        logger->debug("Load data of year {} from database.", year);
        
        remove = std::make_shared<persistence::Data>(year);
        cache.reset();

        countryInfoWidgets.clear();

        if (cache = database.load(year); cache) {
            for (auto it = cache->countries.begin(); it != cache->countries.end(); it++) {
                countryInfoWidgets.emplace_back(it);
            }
        }
    }
    ImGui::SameLine();

    if (cache) {
        if (ImGui::Button("Save")) {
            saveInfoRange(cache->year, cache->year);
        }

        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(POPUP_WINDOW_NAME);
        }
        savePopupWindow();

        ImGui::SameLine();
        helpMarker("Right click \"Save\" button to save for several years.");

        ImGui::SeparatorText("Countries");
        countryInfo();

        ImGui::SeparatorText("Cities");
        cityInfo();

        ImGui::SeparatorText("Note");
        displayNote();
    }
}

void HistoricalInfoWidget::countryInfo()
{
    static std::string countryName;

    countryInfoWidgets.remove_if([this](auto& countryInfoWidget){
        bool remove = false;
        if (ImGui::TreeNode((countryInfoWidget.getName() + "##country").c_str())) {
            ImGui::SeparatorText(countryInfoWidget.getName().c_str());
            countryInfoWidget.paint(this->selected);

            if (this->selected) {
                this->logger->trace("Select coordinate lat {}, lon {} for country {}", this->selected->latitude, this->selected->longitude, countryInfoWidget.getName());
            }
            
            if (ImGui::Button("Delete country")) {
                this->remove->countries.emplace_back(*countryInfoWidget.getCountryIterator());
                this->cache->countries.erase(countryInfoWidget.getCountryIterator());
                this->logger->debug("Delete country {}, current country num in cache: {}", countryInfoWidget.getName(), this->cache->countries.size());
                remove = true;
            }
            ImGui::TreePop();
            ImGui::Spacing();
        }

        return remove;
    });
    ImGui::PushItemWidth(NAME_INPUT_WIDTH);
    ImGui::InputTextWithHint("##Country name", "Country name", &countryName);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button("Add country") && !countryName.empty()) {
        for (const auto& country : cache->countries) {
            if (countryName == country.name) {
                logger->error("Failed to add a new country {}: country already exists in year {}.", countryName, cache->year);
                return;
            }
        }

        cache->countries.emplace_back(countryName, std::list<persistence::Coordinate>{});
        countryInfoWidgets.emplace_back(--cache->countries.end());
        logger->debug("Add country {}, current country num in cache: {}", countryName, this->cache->countries.size());
        countryName.clear();
    }
}

std::vector<HistoricalInfo> HistoricalInfoWidget::getInfo()
{
    return {std::make_tuple("Historical Info", cache, selected)};
}

void HistoricalInfoWidget::drawRightClickMenu(float longitude, float latitude)
{
    if (ImGui::BeginMenu("Add to"))
    {
        if (cache) {
            for (auto& country : cache->countries) {
                if (ImGui::MenuItem(country.name.c_str())) {
                    country.borderContour.emplace_back(latitude, longitude);
                }
            }
        }

        ImGui::EndMenu();
    }
}

void HistoricalInfoWidget::cityInfo()
{
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);
    cache->cities.remove_if([this](auto& city){
        bool remove = false;
        bool hovered = false;

        if (ImGui::TreeNode((city.name + "##city").c_str())) {
            ImGui::InputFloat("latitude", &city.coordinate.latitude, STEP, STEP_FAST, "%.2f");
            hovered |= ImGui::IsItemHovered();
            ImGui::SameLine();
            ImGui::InputFloat("longitude", &city.coordinate.longitude, STEP, STEP_FAST, "%.2f");
            hovered |= ImGui::IsItemHovered();
            ImGui::SameLine();
            if (ImGui::Button("Remove")) {
                this->remove->cities.emplace_back(city);
                this->logger->debug("Delete city {}, current city num in cache: {}", city.name, this->cache->cities.size());
                remove = true;
            }

            if (hovered) {
                this->selected = city.coordinate;
            }

            ImGui::TreePop();
            ImGui::Spacing();
        }
        
        return remove;
    });
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(NAME_INPUT_WIDTH);
    ImGui::InputTextWithHint("##City name", "City name", &newCityName);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);
    ImGui::InputTextWithHint("##CityLatitude" ,"Lat", &cityLatitude);
    ImGui::SameLine();
    ImGui::InputTextWithHint("##CityLongitude", "Lon", &cityLongitude);
    ImGui::PopItemWidth();
    if (ImGui::Button("Add city") && !newCityName.empty() && !cityLongitude.empty() && !cityLatitude.empty()) {
        float lat, lon;
        try {
            lat = std::stod(cityLatitude);
            lon = std::stod(cityLongitude);
        }
        catch (const std::exception &exc) {
            logger->error("Invalid value of new coordinate for city {}.", newCityName);
            return;
        }

        for (const auto& city : cache->cities) {
            if (newCityName == city.name) {
                logger->error("Failed to add a new city {}: city already exists in year {}.", newCityName, cache->year);
                return;
            }
        }

        cache->cities.emplace_back(newCityName, persistence::Coordinate{lat, lon});
        logger->debug("Add city {}, current city num in cache: {}", newCityName, this->cache->cities.size());
        newCityName.clear();
        cityLongitude.clear();
        cityLatitude.clear();
    }
}

void HistoricalInfoWidget::displayNote()
{
    if (ImGui::Button("Clear")) {
        cache->note.text.clear();
    }

    ImGui::InputTextMultiline("##note", &cache->note.text, ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_AllowTabInput);
}

void HistoricalInfoWidget::saveInfo(int saveForYear)
{
    std::shared_ptr<persistence::Data> toUpdate;
    std::shared_ptr<persistence::Data> toRemove;

    if (saveForYear == year) {
        toUpdate = cache;
        toRemove = remove;
    } else {
        toUpdate = std::make_shared<persistence::Data>(*cache);
        toRemove = std::make_shared<persistence::Data>(*remove);

        toUpdate->year = saveForYear;
        toRemove->year = saveForYear;
    }

    logger->debug("Remove {} countries, {} cities, {} event for year {}", toRemove->countries.size(), toRemove->cities.size(), toRemove->note.text, saveForYear);
    logger->debug("Save {} countries, {} cities, {} event for year {}", toUpdate->countries.size(), toUpdate->cities.size(), toUpdate->note.text, saveForYear);
    database.remove(toRemove);
    database.update(toUpdate);
}

void HistoricalInfoWidget::saveInfoRange(int startYear, int endYear)
{
    for (int y = startYear; y <= endYear; y++) {
        saveInfo(y);
    }

    // don't clear cache because the user may continue editing
    remove = std::make_shared<persistence::Data>(year);

    totalWorkLoad = static_cast<float>(database.getWorkLoad());
}

void HistoricalInfoWidget::savePopupWindow()
{
    if (ImGui::BeginPopup(POPUP_WINDOW_NAME)) {
        ImGui::SeparatorText(POPUP_WINDOW_NAME);
        ImGui::InputInt("Start", &startYear);
        ImGui::InputInt("End", &endYear);
        
        if (ImGui::Button("Save##ForYears")) {
            if (startYear <= year && endYear >= year) {
                saveInfoRange(startYear, endYear);
            } else {
                logger->error("Year range must contain current year {}", year);
            }
        }
        
        ImGui::EndPopup();
    }
}

void HistoricalInfoWidget::displaySaveProgress()
{
    const auto left = database.getWorkLoad();
    ImGui::ProgressBar((totalWorkLoad - left) / totalWorkLoad, ImVec2(0.f, 0.f));
    logger->trace("Total workload {}, current left {}", totalWorkLoad, left);

    if (database.getWorkLoad() == 0) {
        totalWorkLoad  = 0;
    }
}

}