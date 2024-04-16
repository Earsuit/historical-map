#ifndef SRC_UI_UTIL_H
#define SRC_UI_UTIL_H

#include "src/persistence/Data.h"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <optional>
#include <type_traits>

namespace ui {
void helpMarker(const char* message);

// we are not so care about the error of floating comparison 
template<typename T>
bool inBound(T v, T min, T max)
{
    return (v > min) && (v < max);
}

void alignForWidth(float width, float alignment = 0.5f);

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

    return selected;
}
}

#endif
