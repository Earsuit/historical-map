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
    ImGui::SliderInt("##", &year, MIN_YEAR, MAX_YEAR, "Year %d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SameLine();
    if (ImGui::Button("-")) {
        year--;
    }
    ImGui::SameLine();
    if (ImGui::Button("+")) {
        year++;
    }
    if (yearLock) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Lock", &yearLock);

    if (yearLock) {
        historyInfo();
    }

    ImGui::End();
}

void HistoricalInfoWidget::historyInfo()
{
    if (ImGui::Button("Refresh") || cache.year != year) {
        logger->debug("Load data of year {} from database.", year);
        cache = persistence.load(year);

        countryInfoWidgets.clear();

        for (auto it = cache.countries.begin(); it != cache.countries.end(); it++) {
            countryInfoWidgets.emplace_back(it);
        }
    }

    ImGui::SeparatorText("Event");
    if (cache.event) {
    }

    ImGui::SeparatorText("Countries");
    countryInfo();

    ImGui::SeparatorText("Cities");
    for (auto& city : cache.cities) {
        if (ImGui::TreeNode(city.name.c_str())) {
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
                this->cache.countries.erase(countryInfoWidget.getCountryIterator());
                this->logger->debug("Delete country {}, current country num in cache: {}", name, this->cache.countries.size());
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
        cache.countries.emplace_back(countryName, std::list<persistence::Coordinate>{});
        countryInfoWidgets.emplace_back(--cache.countries.end());
        logger->debug("Add country {}, current country num in cache: {}", countryName, this->cache.countries.size());
        countryName.clear();
    }
}

}