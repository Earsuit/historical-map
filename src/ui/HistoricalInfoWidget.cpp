#include "src/ui/HistoricalInfoWidget.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

namespace ui {
constexpr int MIN_YEAR = -3000;
constexpr int MAX_YEAR = 1911;
constexpr int SLIDER_WIDTH = 40;
constexpr int COUNTRY_NAME_INPUT_WIDTH = 100;

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
        for (auto& city : cache->cities) {
            if (ImGui::TreeNode(city.name.c_str())) {
            }
        }
    }
}

void HistoricalInfoWidget::countryInfo()
{
    static std::string countryName;

    countryInfoWidgets.remove_if([this](auto& countryInfoWidget){
        const auto name = countryInfoWidget.getName().c_str();
        bool remove = false;
        if (ImGui::TreeNode(name)) {
            ImGui::SeparatorText(name);
            countryInfoWidget.paint(this->selected);

            if (this->selected) {
                this->logger->trace("Select coordinate lat {}, lon {} for country {}", this->selected->latitude, this->selected->longitude, name);
            }
            
            if (ImGui::Button("Delete country")) {
                this->remove->countries.emplace_back(*countryInfoWidget.getCountryIterator());
                this->cache->countries.erase(countryInfoWidget.getCountryIterator());
                this->logger->debug("Delete country {}, current country num in cache: {}", name, this->cache->countries.size());
                remove = true;
            }
            ImGui::TreePop();
            ImGui::Spacing();
        }

        return remove;
    });
    ImGui::PushItemWidth(COUNTRY_NAME_INPUT_WIDTH);
    ImGui::InputText("Country name", &countryName);
    ImGui::SameLine();
    if (ImGui::Button("Add country") && !countryName.empty()) {
        cache->countries.emplace_back(countryName, std::list<persistence::Coordinate>{});
        countryInfoWidgets.emplace_back(--cache->countries.end());
        logger->debug("Add country {}, current country num in cache: {}", countryName, this->cache->countries.size());
        countryName.clear();
    }
}

std::shared_ptr<persistence::Data> HistoricalInfoWidget::getInfo()
{
    return cache;
}

}