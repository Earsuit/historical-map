#include "src/ui/MapWidget.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"

#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>

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

constexpr float POINT_SIZE = 2.0f;
constexpr float SELECTED_SOINT_SIZE = 4.0f;
constexpr int LINE_OFFSET = 0;

constexpr int FILLED_ALPHA = 50;
constexpr auto NORMALIZE = 255.0f; 
constexpr uint8_t MASK = 0xFF; 
constexpr auto DEFAULT_ALPHA = 1.0f;

ImVec4 computeColor(const std::string& val)
{
    const auto hash = std::hash<std::string>{}(val);

    float r = static_cast<float>((hash & MASK) / NORMALIZE);
    float g = static_cast<float>(((hash >> 8) & MASK) / NORMALIZE);
    float b = static_cast<float>(((hash >> 16) & MASK) / NORMALIZE);

    return ImVec4(r, g, b, DEFAULT_ALPHA);
}

void MapWidget::paint()
{
    renderTile();
    renderOverlay();
}

void MapWidget::renderTile()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(0, 0));
    ImGui::Begin(MAP_WIDGET_NAME);
    ImGui::PopStyleVar(2);
    
    const auto sizeAvail = ImGui::GetContentRegionAvail();
    if (ImPlot::BeginPlot("##map", ImVec2(sizeAvail.x, sizeAvail.y), ImPlotFlags_CanvasOnly)) {
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_NoButtons);
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
        const auto xMin = std::clamp(static_cast<int>(tile::longitude2X(west, zoom)), 0, limit);
        const auto xMax = std::clamp(static_cast<int>(tile::longitude2X(east, zoom)), 0, limit);
        const auto yMin = std::clamp(static_cast<int>(tile::latitude2Y(north, zoom)), 0, limit);
        const auto yMax = std::clamp(static_cast<int>(tile::latitude2Y(south, zoom)), 0, limit);

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

        renderHistoricalInfo();

        ImPlot::EndPlot();
    }

    if (ImGui::BeginPopupContextItem("##map")) {
        static float longitue = 0, latitude = 0;
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            longitue = tile::x2Longitude(mousePos.x, BBOX_ZOOM_LEVEL);
            latitude = tile::y2Latitude(mousePos.y, BBOX_ZOOM_LEVEL);
        }

        historicalInfoWidget.drawRightClickMenu(longitue, latitude);
        ImGui::EndPopup();
    }

    ImGui::End();
}

void MapWidget::renderOverlay()
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

void MapWidget::renderHistoricalInfo()
{
    auto [data, selected] = historicalInfoWidget.getInfo();

    if (data) {
        int dragPointId = 0;
        for (auto& country : data->countries) {
            std::vector<ImVec2> points;
            const auto color = computeColor(country.name);

            points.reserve(country.borderContour.size());
            for (auto& coordinate : country.borderContour) {
                float size = POINT_SIZE;

                if (selected && coordinate == *selected) {
                    size = SELECTED_SOINT_SIZE;
                }

                auto [x, y] = renderCoordinate(coordinate, color, size, dragPointId++);
                points.emplace_back(ImPlot::PlotToPixels(ImPlotPoint(x, y)));
            }

            ImPlot::GetPlotDrawList()->AddConvexPolyFilled(points.data(), points.size(), IM_COL32(color.x * NORMALIZE, color.y * NORMALIZE, color.z * NORMALIZE, FILLED_ALPHA));
        }
    }
}

ImVec2 MapWidget::renderCoordinate(persistence::Coordinate& coordinate, const ImVec4& color, float size, int id)
{
    double x = tile::longitude2X(coordinate.longitude, BBOX_ZOOM_LEVEL);
    double y = tile::latitude2Y(coordinate.latitude, BBOX_ZOOM_LEVEL);
    
    if (ImPlot::DragPoint(id, &x, &y, color, size)) {
        logger->trace("Lon {}, lat {} at [{}, {}]",
                    coordinate.longitude,
                    coordinate.latitude,
                    x,
                    y);
        coordinate.latitude = tile::y2Latitude(static_cast<float>(y), BBOX_ZOOM_LEVEL);
        coordinate.longitude = tile::x2Longitude(static_cast<float>(x), BBOX_ZOOM_LEVEL);
    }

    return {static_cast<float>(x) , static_cast<float>(y)};
}

tile::TileLoader& MapWidget::getTileLoader()
{
    return tileLoader;
}

}