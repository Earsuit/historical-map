#include "src/ui/MapWidget.h"

#include "external/imgui/imgui.h"
#include "external/implot/implot.h"

#include <cmath>

namespace ui {

constexpr auto AXIS_FLAGS = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoGridLines |
                       ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels |
                       ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoMenus |
                       ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoHighlight;

void MapWidget::paint()
{
    ImGui::Begin("Map plot");
    if (ImPlot::BeginPlot("##map")) {
        ImPlot::SetupAxis(ImAxis_X1, nullptr, AXIS_FLAGS);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, AXIS_FLAGS | ImPlotAxisFlags_Invert);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0.0, 1.0);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, 1.0);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, 1.0);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1.0);

        ImPlot::EndPlot();
    }

    ImGui::End();
}

// uv0, uv1
std::pair<ImVec2, ImVec2> MapWidget::calculateBound(int x, int y)
{
    return {};
}

}