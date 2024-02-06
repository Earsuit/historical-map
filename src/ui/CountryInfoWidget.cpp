#include "src/ui/CountryInfoWidget.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

#include <type_traits>

namespace ui {
constexpr int COORDINATE_INPUT_WIDTH = 50;

namespace {
constexpr double STEP = 0;
constexpr double STEP_FAST = 0;

template<typename T>
void inputFiled(const char* label, T* value)
{
    if constexpr (std::is_same_v<T, float>) {
        ImGui::InputFloat(label, value, STEP, STEP_FAST, "%.2f");
    } else {
        ImGui::InputText(label, value);
    }
}
}

void CountryInfoWidget::paint(std::optional<persistence::Coordinate>& selected)
{
    bool hovered = false;

    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);
    country.borderContour.erase(std::remove_if(country.borderContour.begin(), country.borderContour.end(), [&selected, this](auto& coordinate){
                                                    bool hovered = false;
                                                    bool remove = false;

                                                    // create ID scope so we can reuse labels
                                                    ImGui::PushID(&coordinate);
                                                    inputFiled("latitude", &coordinate.latitude);
                                                    hovered |= ImGui::IsItemHovered();
                                                    ImGui::SameLine();
                                                    inputFiled("longitude", &coordinate.longitude);
                                                    hovered |= ImGui::IsItemHovered();
                                                    ImGui::SameLine();

                                                    if (ImGui::Button("Remove")) {
                                                        this->logger->debug("Delete coordinate lat={}, lon={}.", coordinate.latitude, coordinate.longitude);
                                                        remove = true;
                                                    }

                                                    if (hovered) {
                                                        selected = coordinate;
                                                    }

                                                    ImGui::PopID();

                                                    return remove;
                                                }),
                                country.borderContour.end());

    // input filed for new coordinate
    inputFiled("Latitude", &latitude);
    hovered |= ImGui::IsItemHovered();
    ImGui::SameLine();
    inputFiled("Longitude", &longitude);
    hovered |= ImGui::IsItemHovered();
    ImGui::SameLine();
    const auto pressed = ImGui::Button("Add");
    ImGui::PopItemWidth();

    if (!latitude.empty() && !longitude.empty()) {
        float lat, lon;
        try {
            lat = std::stod(latitude);
            lon = std::stod(longitude);
        }
        catch (const std::exception &exc) {
            logger->error("Invalid value for new coordinate for country {}.", country.name);
            return;
        }
        persistence::Coordinate newCorrd{lat, lon};

        if (hovered) {
            selected = newCorrd;
        }

        if (pressed) {
            logger->debug("Add coordinate lat={}, lon={}.", latitude, longitude);
            country.borderContour.emplace_back(newCorrd);
            latitude.clear();
            longitude.clear();
        }
    }
}

std::string CountryInfoWidget::getName()
{
    return country.name;
}
}