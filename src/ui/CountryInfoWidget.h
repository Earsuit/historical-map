#ifndef SRC_UI_COUNTRY_INFO_WIDGET_H
#define SRC_UI_COUNTRY_INFO_WIDGET_H

#include "src/persistence/Data.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"
#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

#include <string>
#include <optional>
#include <memory>
#include <vector>

namespace ui {
class CountryInfoWidget {
public:
    CountryInfoWidget(std::list<persistence::Country>::iterator it, bool immutable):
        logger{spdlog::get(logger::LOGGER_NAME)},
        it{it},
        country{*it},
        immutable{immutable}
    {
    }

    void paint(std::optional<persistence::Coordinate>& selected);

    std::string getName();

    auto getCountryIterator() { return it; }

private:
    std::shared_ptr<spdlog::logger> logger;
    std::list<persistence::Country>::iterator it;
    persistence::Country& country;
    bool immutable;
    std::string latitude{};
    std::string longitude{};

    constexpr static double STEP = 0;
    constexpr static double STEP_FAST = 0;

    template<typename T>
    void inputFiled(const char* label, T* value)
    {
        ImGuiInputTextFlags flag = immutable ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None;
        if constexpr (std::is_same_v<T, float>) {
            ImGui::InputFloat(label, value, STEP, STEP_FAST, "%.2f", flag);
        } else {
            ImGui::InputText(label, value, flag);
        }
    }
};
}

#endif
