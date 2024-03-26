#include "src/ui/IInfoWidget.h"
#include "src/ui/Util.h"

#include "external/imgui/imgui.h"

namespace ui {
constexpr int MIN_YEAR = -3000;
constexpr int MAX_YEAR = 1911;

void IInfoWidget::paint()
{
    ImGui::Begin(INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar);

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

    historyInfo();

    ImGui::End();
}
}