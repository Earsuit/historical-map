#include "src/ui/MapWidget.h"

#include "external/imgui/imgui.h"

#include <cmath>
#include <algorithm>

namespace ui {

constexpr auto AXIS_FLAGS = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoGridLines |
                            ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels |
                            ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoMenus |
                            ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoHighlight;
constexpr int BBOX_ZOOM_LEVEL = 0; // only compute the zoom level from level 0, otherwise the computed zoom level will be a mess
constexpr ImVec4 OVERLAY_BACKGROUND_COLOR = {0.0f, 0.0f, 0.0f, 0.35f};
constexpr float OVERLAY_PAD = 10.0f;
constexpr float MAX_LONGITUDE = 180.0f;
constexpr float MIN_LONGITUDE = -180.0f;
constexpr float MAX_LATITUDE = 85.05112878f;
constexpr float MIN_LATITUDE = -85.05112878f;

void MapWidget::paint()
{
    renderTile();
    overlay();
}

void MapWidget::setTileSource(std::shared_ptr<tile::TileSource> tileSource)
{
    tileLoader.setTileSource(tileSource);
}

void MapWidget::renderTile()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(0, 0));
    ImGui::Begin(MAP_WIDGET_NAME, nullptr,  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar(2);
    
    const auto sizeAvail = ImGui::GetContentRegionAvail();
    if (ImPlot::BeginPlot("##map", ImVec2(sizeAvail.x, sizeAvail.y), ImPlotFlags_NoFrame)) {
        ImPlot::SetupAxis(ImAxis_X1, nullptr, AXIS_FLAGS);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, AXIS_FLAGS | ImPlotAxisFlags_Invert);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0.0, 1.0);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0.0, 1.0);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, 1.0);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 1.0);
        
        const auto plotLimits = ImPlot::GetPlotLimits(ImAxis_X1, ImAxis_Y1);
        const auto plotSize = ImPlot::GetPlotSize();
        mousePos = ImPlot::GetPlotMousePos();

        const auto west = tile::x2Longitude(plotLimits.X.Min, BBOX_ZOOM_LEVEL);
        const auto east = tile::x2Longitude(plotLimits.X.Max, BBOX_ZOOM_LEVEL);
        const auto north = tile::y2Latitude(plotLimits.Y.Min, BBOX_ZOOM_LEVEL);
        const auto south = tile::y2Latitude(plotLimits.Y.Max, BBOX_ZOOM_LEVEL);
        bbox = {west, south, east, north};

        logger->trace("Plot limit X [{}, {}], Y [{}, {}]", plotLimits.X.Min, plotLimits.X.Max, plotLimits.Y.Min, plotLimits.Y.Max);
        logger->trace("Plot size x={}, y={} pixels", plotSize.x, plotSize.y);
        logger->trace("west={}, north={}, east={}, south={}", west, north, east, south);

        zoom = std::clamp(bestZoomLevel(bbox, 0, plotSize.x, plotSize.y), tile::MIN_ZOOM_LEVEL, tile::MAX_ZOOM_LEVEL);

        const auto limit = (1 << zoom) - 1;
        const auto xMin = std::clamp(tile::longitude2X(west, zoom), 0, limit);
        const auto xMax = std::clamp(tile::longitude2X(east, zoom), 0, limit);
        const auto yMin = std::clamp(tile::latitude2Y(north, zoom), 0, limit);
        const auto yMax = std::clamp(tile::latitude2Y(south, zoom), 0, limit);

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

void MapWidget::overlay()
{
    static bool firstTime = true;
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | 
                                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    if (firstTime) {
        firstTime = false;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = work_pos.x + OVERLAY_PAD;
        window_pos.y = work_pos.y + OVERLAY_PAD;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
    }

    ImGui::PushStyleColor(ImGuiCol_TitleBg, OVERLAY_BACKGROUND_COLOR);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, OVERLAY_BACKGROUND_COLOR);
    if (ImGui::Begin("##Overlay", nullptr, windowFlags)) {
        ImGui::Text("FPS: %.f", ImGui::GetIO().Framerate);
        ImGui::Text("Cursor at: lon %.2f, lat %.2f", std::clamp(tile::x2Longitude(mousePos.x, BBOX_ZOOM_LEVEL), MIN_LONGITUDE, MAX_LONGITUDE), 
                                                     std::clamp(tile::y2Latitude(mousePos.y, BBOX_ZOOM_LEVEL), MIN_LATITUDE, MAX_LATITUDE));
        ImGui::Text("View range: west %.2f, east %.2f, \n\t\t\tnorth %.2f, south %.2f", bbox.west, bbox.east, bbox.north, bbox.south);
        
    }
    ImGui::PopStyleColor(2);
    ImGui::End();
}

}