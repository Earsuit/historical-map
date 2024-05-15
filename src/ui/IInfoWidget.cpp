#include "src/ui/IInfoWidget.h"
#include "src/ui/Util.h"

#include "external/imgui/imgui.h"

namespace ui {
void IInfoWidget::paint()
{
    int year = presenter.getYear();
    ImGui::Begin(INFO_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar);

    // there is no year 0
    if (ImGui::SliderInt("##", &year, presenter.getMinYear(), presenter.getMaxYear(), "Year %d", ImGuiSliderFlags_AlwaysClamp)) {
        presenter.handleSetYear(year);
    }
    
    ImGui::SameLine();
    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        presenter.handleMoveYearBackward();
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
        presenter.handleMoveYearForward();
    }
    ImGui::PopButtonRepeat();

    ImGui::SameLine();
    helpMarker("Ctrl + click to maually set the year");

    historyInfo(year);

    ImGui::End();
}
}