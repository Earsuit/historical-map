#ifndef SRC_UI_COUNTRY_INFO_WIDGET_H
#define SRC_UI_COUNTRY_INFO_WIDGET_H

#include "src/persistence/Data.h"
#include "src/logger/Util.h"
#include "src/util/TypeTraits.h"

#include "spdlog/spdlog.h"
#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

#include <string>
#include <optional>
#include <memory>

namespace ui {
template<typename T>
class CountryInfoWidget {
public:
    CountryInfoWidget(T it):
        logger{spdlog::get(logger::LOGGER_NAME)},
        it{it},
        country{*it}
    {
    }

    void paint(std::optional<persistence::Coordinate>& selected)
    {
        bool hovered = false;

        ImGui::PushItemWidth(COORDINATE_INPUT_WIDTH);

        auto loopFunc = [&selected, this](auto& coordinate){
                            bool hovered = false;
                            bool remove = false;

                            // create ID scope so we can reuse labels
                            ImGui::PushID(&coordinate);
                            this->inputFiled("latitude", coordinate.latitude);
                            hovered |= ImGui::IsItemHovered();
                            ImGui::SameLine();
                            this->inputFiled("longitude", coordinate.longitude);
                            hovered |= ImGui::IsItemHovered();

                            if constexpr (!util::is_const_iterator_v<T>) {
                                ImGui::SameLine();
                                if (ImGui::Button("Remove")) {
                                    this->logger->debug("Delete coordinate lat={}, lon={}.", coordinate.latitude, coordinate.longitude);
                                    remove = true;
                                }
                            }

                            if (hovered) {
                                selected = coordinate;
                            }

                            ImGui::PopID();

                            return remove;
                        };

        if constexpr (util::is_const_iterator_v<T>) {
            std::for_each(country.borderContour.cbegin(), country.borderContour.cend(), loopFunc);
        } else {
            country.borderContour.erase(std::remove_if(country.borderContour.begin(), country.borderContour.end(), loopFunc), 
                                        country.borderContour.end());

            // input filed for new coordinate
            inputFiled("Latitude", latitude);
            hovered |= ImGui::IsItemHovered();
            ImGui::SameLine();
            inputFiled("Longitude", longitude);
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
    }

    std::string getName() const noexcept { return country.name; }

    auto getCountryIterator() noexcept { return it; }

private:
    std::shared_ptr<spdlog::logger> logger;
    T it;
    decltype(*it) country;
    std::string latitude{};
    std::string longitude{};

    constexpr static double STEP = 0;
    constexpr static double STEP_FAST = 0;
    constexpr static int COORDINATE_INPUT_WIDTH = 50;

    template<typename Y>
    void inputFiled(const char* label, Y& value)
    {
        if constexpr (std::is_same_v<std::remove_const_t<Y>, float>) {
            ImGui::InputFloat(label, &value, STEP, STEP_FAST, "%.2f");
        } else {
            ImGui::InputText(label, &value);
        }
    }

    template<typename Y>
    void inputFiled(const char* label, const Y& value)
    {
        if constexpr (std::is_same_v<Y, float>) {
            ImGui::LabelText(label, "%.2f", value);
        } else {
            ImGui::LabelText(label, value);
        }
    }
};
}

#endif
