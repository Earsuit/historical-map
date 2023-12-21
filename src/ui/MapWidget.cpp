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
    const auto sizeAvail = ImGui::GetContentRegionAvail();
    if (ImPlot::BeginPlot("##map", ImVec2(sizeAvail.x, sizeAvail.y))) {
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

        logger->trace("Plot limit X [{}, {}], Y [{}, {}]", plotLimits.X.Min, plotLimits.X.Max, plotLimits.Y.Min, plotLimits.Y.Max);
        logger->trace("Plot size x={}, y={} pixels", plotSize.x, plotSize.y);
        logger->trace("west={}, north={}, east={}, south={}", west, north, east, south);

        zoom = bestZoomLevel(bbox, 0, plotSize.x, plotSize.y);

        int xMin = 0, xMax = 0, yMin = 0, yMax = 0;

        // if zoom == 0, there is only one tile [0,0]
        // but if use longitude2X or latitude2Y, the xMax and yMax will be 1 due to the formula
        // so we handle them differently
        if (zoom != 0) {
            xMin = tile::longitude2X(west, zoom);
            xMax = tile::longitude2X(east, zoom);
            yMin = tile::latitude2Y(north, zoom);
            yMax = tile::latitude2Y(south, zoom);
        }

        logger->trace("Zoom {} tile X from [{}, {}], Y from [{}, {}]", zoom, xMin, xMax, yMin, yMax);

        ImVec2 bMax = {0, 0};
        ImVec2 bMin = {0, 0};

        for (auto x = xMin; x <= xMax; x++) {
            bMax.x = tile::computeTileBound(x+1, zoom);
            bMin.x = tile::computeTileBound(x, zoom);
            for (auto y = yMin; y <= yMax; y++) {
                bMax.y = tile::computeTileBound(y+1, zoom);
                bMin.y = tile::computeTileBound(y, zoom);
                if (auto tile = tileLoader.loadTile({x, y, zoom}); tile) {
                    ImPlot::PlotImage("##", (*tile)->getTexture(), bMin, bMax);
                }
            }
        }

        ImPlot::EndPlot();
    }

    ImGui::End();
}

// uv0, uv1
std::pair<ImVec2, ImVec2> MapWidget::calculateBound(int x, int y)
{
    return {};
}

void MapWidget::setTileSource(std::shared_ptr<tile::TileSource> tileSource)
{
    tileLoader.setTileSource(tileSource);
}

}