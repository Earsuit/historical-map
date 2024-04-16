#ifndef SRC_UI_UTIL_H
#define SRC_UI_UTIL_H

#include "src/persistence/Data.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <optional>
#include <type_traits>

namespace ui {
constexpr int COORDINATE_INPUT_WIDTH = 50;
 
void helpMarker(const char* message);
void alignForWidth(float width, float alignment = 0.5f);

// we are not so care about the error of floating comparison 
template<typename T>
bool inBound(T v, T min, T max)
{
    return (v > min) && (v < max);
}

template<typename Y>
void itemFiled(const char* label, Y& value)
{
    constexpr double STEP = 0;
    constexpr double STEP_FAST = 0;

    if constexpr (std::is_same_v<std::remove_const_t<Y>, float>) {
        ImGui::InputFloat(label, &value, STEP, STEP_FAST, "%.2f");
    } else {
        ImGui::InputText(label, &value);
    }
}

template<typename Y>
void itemFiled(const char* label, const Y& value)
{
    if constexpr (std::is_same_v<Y, float>) {
        ImGui::LabelText(label, "%.2f", value);
    } else {
        ImGui::LabelText(label, value);
    }
}

template<typename T>
std::optional<persistence::Coordinate> paintCountryInfo(T& country)
{
    std::optional<persistence::Coordinate> selected;

    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);
    auto loopFunc = [&selected](auto& coordinate){
                        bool hovered = false;
                        bool remove = false;

                        // create ID scope so we can reuse labels
                        ImGui::PushID(&coordinate);
                        itemFiled("latitude", coordinate.latitude);
                        hovered |= ImGui::IsItemHovered();
                        ImGui::SameLine();
                        itemFiled("longitude", coordinate.longitude);
                        hovered |= ImGui::IsItemHovered();

                        if constexpr (!std::is_const_v<std::remove_reference_t<T>>) {
                            ImGui::SameLine();
                            if (ImGui::Button("Remove")) {
                                remove = true;
                            }
                        }

                        if (hovered) {
                            selected = coordinate;
                        }

                        ImGui::PopID();

                        return remove;
                    };

    if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
        std::for_each(country.borderContour.cbegin(), country.borderContour.cend(), loopFunc);
    } else {
        country.borderContour.erase(std::remove_if(country.borderContour.begin(), country.borderContour.end(), loopFunc), 
                                    country.borderContour.cend());
    }

    ImGui::PopItemWidth();

    return selected;
}

template<typename T>
std::optional<persistence::Coordinate> paintCityInfo(T& city)
{
    std::optional<persistence::Coordinate> selected;
    bool hovered = false;

    ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);

    itemFiled("latitude", city.coordinate.latitude);
    hovered |= ImGui::IsItemHovered();
    ImGui::SameLine();
    itemFiled("longitude", city.coordinate.longitude);
    hovered |= ImGui::IsItemHovered();

    if (hovered) {
        selected = city.coordinate;
    }

    ImGui::PopItemWidth();

    return selected;
}
}

#endif
