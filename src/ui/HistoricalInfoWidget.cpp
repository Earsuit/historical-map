#include "src/ui/HistoricalInfoWidget.h"

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

void HistoricalInfoWidget::paint()
{
    ImGui::Begin(HISTORICAL_INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar);
    
    if (yearLock) {
        ImGui::BeginDisabled();
    }
    // there is no year 0
    ImGui::SliderInt("##", &year, MIN_YEAR, MAX_YEAR, "Year %d", ImGuiSliderFlags_AlwaysClamp);
    if (year == 0) {
        year = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("-")) {
        year--;
        if (year == 0) {
            year = -1;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("+")) {
        year++;
        if (year == 0) {
            year = 1;
        }
    }
    if (yearLock) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Lock", &yearLock);

    if (yearLock) {
        historyInfo();
    } else {
        cache.reset();
        remove = std::make_shared<persistence::Data>(year);
    }

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

        if (cache = persistence.load(year); cache) {
            for (auto it = cache->countries.begin(); it != cache->countries.end(); it++) {
                countryInfoWidgets.emplace_back(it);
            }
        }
    }
    ImGui::SameLine();

    if (cache) {
        if (ImGui::Button("Save")) {
            logger->debug("Remove {} countries, {} cities, {} event", remove->countries.size(), remove->cities.size(), static_cast<bool>(remove->event));
            logger->debug("Save {} countries, {} cities, {} event", cache->countries.size(), cache->cities.size(), static_cast<bool>(cache->event));
            persistence.remove(remove);
            persistence.update(cache);

            // don't clear cache because the user may continue editing
            remove = std::make_shared<persistence::Data>(remove->year);
        }

        ImGui::SeparatorText("Event");
        if (cache->event) {
        }

        ImGui::SeparatorText("Countries");
        countryInfo();

        ImGui::SeparatorText("Cities");
        cityInfo();
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

std::pair<std::shared_ptr<persistence::Data>, std::optional<persistence::Coordinate>> HistoricalInfoWidget::getInfo()
{
    return {cache, selected};
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
    cache->cities.remove_if([this](auto& city){
        bool remove = false;

        if (ImGui::TreeNode((city.name + "##city").c_str())) {
            ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);
            ImGui::InputFloat("longitude", &city.coordinate.longitude, STEP, STEP_FAST, "%.2f");
            ImGui::SameLine();
            ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);
            ImGui::InputFloat("latitude", &city.coordinate.latitude, STEP, STEP_FAST, "%.2f");
            if (ImGui::Button("Remove")) {
                this->remove->cities.emplace_back(city);
                this->logger->debug("Delete city {}, current city num in cache: {}", city.name, this->cache->cities.size());
                remove = true;
            }

            ImGui::TreePop();
            ImGui::Spacing();
        }
        
        return remove;
    });

    ImGui::PushItemWidth(NAME_INPUT_WIDTH);
    ImGui::InputTextWithHint("##City name", "City name", &newCityName);
    ImGui::SameLine();
    if (ImGui::Button("Add city") && !newCityName.empty()) {
        for (const auto& city : cache->cities) {
            if (newCityName == city.name) {
                logger->error("Failed to add a new city {}: city already exists in year {}.", newCityName, cache->year);
                return;
            }
        }

        cache->cities.emplace_back(newCityName, persistence::Coordinate{});
        logger->debug("Add city {}, current city num in cache: {}", newCityName, this->cache->cities.size());
        newCityName.clear();
    }
}

}