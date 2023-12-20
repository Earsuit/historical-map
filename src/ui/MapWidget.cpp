#include "src/ui/MapWidget.h"
#include "src/tile/Util.h"

#include "external/imgui/imgui.h"
#include "external/implot/implot.h"

#include <cmath>

namespace ui {

constexpr auto AXIS_FLAGS = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoGridLines |
                       ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels |
                       ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoMenus |
                       ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoHighlight;
constexpr int BBOX_ZOOM_LEVEL = 0; // only compute the zoom level from level 0, otherwise the computed zoom level will be a mess

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
        
        const auto plotLimits = ImPlot::GetPlotLimits(ImAxis_X1, ImAxis_Y1);
        const auto plotSize = ImPlot::GetPlotSize();

        const auto west = tile::x2Longitude(plotLimits.X.Min, BBOX_ZOOM_LEVEL);
        const auto east = tile::x2Longitude(plotLimits.X.Max, BBOX_ZOOM_LEVEL);
        const auto north = tile::y2Latitude(plotLimits.Y.Min, BBOX_ZOOM_LEVEL);
        const auto south = tile::y2Latitude(plotLimits.Y.Max, BBOX_ZOOM_LEVEL);
        const tile::BoundingBox bbox = {west, south, east, north};

        logger.debug("Plot limit X [{}, {}], Y [{}, {}]", plotLimits.X.Min, plotLimits.X.Max, plotLimits.Y.Min, plotLimits.Y.Max);
        logger.debug("Plot size x={}, y={} pixels", plotSize.x, plotSize.y);
        logger.debug("west={}, north={}, east={}, south={}", west, north, east, south);

        zoom = bestZoomLevel(bbox, 0, plotSize.x, plotSize.y);
        logger.debug("Zoom {}", zoom);

        ImPlot::EndPlot();
    }

    ImGui::End();
}

// uv0, uv1
std::pair<ImVec2, ImVec2> MapWidget::calculateBound(int x, int y)
{
    return {};
}

void setTileSource(std::shared_ptr<tile::TileSource> tileSource)
{
    tileSource = tileSource;
}

}