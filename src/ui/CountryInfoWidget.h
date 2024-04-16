#ifndef SRC_UI_COUNTRY_INFO_WIDGET_H
#define SRC_UI_COUNTRY_INFO_WIDGET_H

#include "src/persistence/Data.h"
#include "src/logger/Util.h"
#include "src/util/TypeTraits.h"
#include "src/ui/Util.h"

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

        if (const auto& ret = paintCountryInfo(country); ret) {
            // only update if we hovered on this country
            selected = ret;
        }

        if constexpr (!util::is_const_iterator_v<T>) {
            // input filed for new coordinate
            itemFiled("Latitude", latitude);
            hovered |= ImGui::IsItemHovered();
            ImGui::SameLine();
            itemFiled("Longitude", longitude);
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

    constexpr static int COORDINATE_INPUT_WIDTH = 50;
};
}

#endif
