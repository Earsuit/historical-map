#include "src/ui/MapWidget.h"
#include "src/ui/Util.h"

#include "external/imgui/imgui.h"
#include "external/imgui/misc/cpp/imgui_stdlib.h"
#include "external/implot/implot_internal.h"
#include "mapbox/polylabel.hpp"

#include <cmath>
#include <algorithm>
#include <functional>
#include <limits>

namespace ui {

constexpr auto AXIS_FLAGS = ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoGridLines |
                            ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoTickLabels |
                            ImPlotAxisFlags_NoInitialFit | ImPlotAxisFlags_NoMenus |
                            ImPlotAxisFlags_NoMenus | ImPlotAxisFlags_NoHighlight;

constexpr ImVec4 OVERLAY_BACKGROUND_COLOR = {0.0f, 0.0f, 0.0f, 0.35f};
constexpr float OVERLAY_PAD = 10.0f;
constexpr float MAX_LONGITUDE = 180.0f;
constexpr float MIN_LONGITUDE = -180.0f;
constexpr float MAX_LATITUDE = 85.05112878f;
constexpr float MIN_LATITUDE = -85.05112878f;

constexpr float POINT_SIZE = 2.0f;
constexpr float SELECTED_POINT_SIZE = 4.0f;
constexpr auto CITY_ANNOTATION_OFFSET = ImVec2(-15, 15);
constexpr auto COUNTRY_ANNOTATION_OFFSET = ImVec2(0, 0);
constexpr auto VISUAL_CENTER_PERCISION = 1.0;
constexpr auto MINIMAL_POINTS_OF_POLYGON = 3;

constexpr int FILLED_ALPHA = 50;
constexpr auto NORMALIZE = 255.0f; 
constexpr uint8_t MASK = 0xFF; 
constexpr auto DEFAULT_ALPHA = 1.0f;
constexpr auto MIN_AXIS_RANGE = 0.0;
constexpr auto MAX_AXIS_RANGE = 1.0;

constexpr auto INIT_X_LIMIT_MIN = 0.6398;
constexpr auto INIT_X_LIMIT_MAX = 0.9349;
constexpr auto INIT_Y_LIMIT_MIN = 0.3162;
constexpr auto INIT_Y_LIMIT_MAX = 0.4779;

ImVec4 computeColor(const std::string& val)
{
    const auto hash = std::hash<std::string>{}(val);

    float r = static_cast<float>((hash & MASK) / NORMALIZE);
    float g = static_cast<float>(((hash >> 8) & MASK) / NORMALIZE);
    float b = static_cast<float>(((hash >> 16) & MASK) / NORMALIZE);

    return ImVec4(r, g, b, DEFAULT_ALPHA);
}

void MapWidget::paint(IInfoWidget& infoWidget)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(0, 0));
    ImGui::Begin(MAP_WIDGET_NAME);
    ImGui::PopStyleVar(2);

    const auto area = ImGui::GetContentRegionAvail();
    const auto infos = infoWidget.getInfo();

    const ImVec2 singleMapWindowSize = {area.x / infos.size(), area.y};
    int overlayOffset = 0;

    for (auto [name, historicalData, selected] : infos) {
        const auto [bbox, mousePos] = renderMap(infoWidget, singleMapWindowSize, name, historicalData, selected);
        renderOverlay(name, overlayOffset, bbox, mousePos);
        overlayOffset += singleMapWindowSize.x + ImPlot::GetStyle().PlotPadding.x - ImPlot::GetStyle().PlotBorderSize * 2;
        ImGui::SameLine();
    }

    ImGui::End();
}

std::pair<tile::BoundingBox, std::optional<ImPlotPoint>> MapWidget::renderMap(IInfoWidget& infoWidget,
                                                                              ImVec2 size, 
                                                                              const std::string& name, 
                                                                              HistoricalData historicalData, 
                                                                              std::optional<persistence::Coordinate> selected)
{
    tile::BoundingBox bbox;
    std::optional<ImPlotPoint> mousePos;
    std::string plotName = "##" + name; 

    if (ImPlot::BeginPlot(plotName.c_str(), size, (ImPlotFlags_CanvasOnly ^ ImPlotFlags_NoLegend) | ImPlotFlags_Equal)) {
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_NoButtons);
        ImPlot::SetupAxis(ImAxis_X1, nullptr, AXIS_FLAGS);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, AXIS_FLAGS | ImPlotAxisFlags_Invert);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, MIN_AXIS_RANGE, MAX_AXIS_RANGE);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, MIN_AXIS_RANGE, MAX_AXIS_RANGE);
        ImPlot::SetupAxisLimits(ImAxis_X1, INIT_X_LIMIT_MIN, INIT_X_LIMIT_MAX);
        ImPlot::SetupAxisLimits(ImAxis_Y1, INIT_Y_LIMIT_MIN, INIT_Y_LIMIT_MAX);
        
        const auto plotSize = ImPlot::GetPlotSize();
        const auto plotLimits = ImPlot::GetPlotLimits(ImAxis_X1, ImAxis_Y1);
        if (const auto cursor = ImPlot::GetPlotMousePos(); 
            inBound(cursor.x, plotLimits.X.Min, plotLimits.X.Max) &&
            inBound(cursor.y, plotLimits.Y.Min, plotLimits.Y.Max)) {
            mousePos = cursor;
        }

        const auto west = tile::x2Longitude(plotLimits.X.Min, BBOX_ZOOM_LEVEL);
        const auto east = tile::x2Longitude(plotLimits.X.Max, BBOX_ZOOM_LEVEL);
        const auto north = tile::y2Latitude(plotLimits.Y.Min, BBOX_ZOOM_LEVEL);
        const auto south = tile::y2Latitude(plotLimits.Y.Max, BBOX_ZOOM_LEVEL);
        bbox = {west, south, east, north};

        logger->trace("Plot limit X [{}, {}], Y [{}, {}]", plotLimits.X.Min, plotLimits.X.Max, plotLimits.Y.Min, plotLimits.Y.Max);
        logger->trace("Plot size x={}, y={} pixels", plotSize.x, plotSize.y);
        logger->trace("west={}, north={}, east={}, south={}", west, north, east, south);

        const auto zoom = std::clamp(bestZoomLevel(bbox, 0, plotSize.x, plotSize.y), tile::MIN_ZOOM_LEVEL, tile::MAX_ZOOM_LEVEL);

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

        renderHistoricalInfo(historicalData, selected);

        ImPlot::EndPlot();
    }

    if (ImGui::BeginPopupContextItem(plotName.c_str())) {
        static float longitue = 0, latitude = 0;
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && mousePos) {
            longitue = tile::x2Longitude(mousePos->x, BBOX_ZOOM_LEVEL);
            latitude = tile::y2Latitude(mousePos->y, BBOX_ZOOM_LEVEL);
        }

        infoWidget.drawRightClickMenu(longitue, latitude);
        ImGui::EndPopup();
    }

    return {bbox, mousePos};
}

void MapWidget::renderOverlay(const std::string& name, int offset, const tile::BoundingBox& bbox, const std::optional<ImPlotPoint>& mousePos)
{
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | 
                                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 window_pos;
    window_pos.x = work_pos.x + OVERLAY_PAD + offset;
    window_pos.y = work_pos.y + OVERLAY_PAD;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleColor(ImGuiCol_TitleBg, OVERLAY_BACKGROUND_COLOR);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, OVERLAY_BACKGROUND_COLOR);
    if (ImGui::Begin((name + "##Overlay").c_str(), nullptr, windowFlags)) {
        if (mousePos) {
            ImGui::Text("Cursor at: lon %.2f, lat %.2f", 
                        std::clamp(tile::x2Longitude(mousePos->x, BBOX_ZOOM_LEVEL), MIN_LONGITUDE, MAX_LONGITUDE), 
                        std::clamp(tile::y2Latitude(mousePos->y, BBOX_ZOOM_LEVEL), MIN_LATITUDE, MAX_LATITUDE));
        } else {
            ImGui::Text("Cursor at: ");
        }

        ImGui::Text("View range: west %.2f, east %.2f, \n\t\t\tnorth %.2f, south %.2f", bbox.west, bbox.east, bbox.north, bbox.south);
    }
    ImGui::PopStyleColor(2);
    ImGui::End();
}

void MapWidget::renderHistoricalInfo(HistoricalData historicalData, std::optional<persistence::Coordinate> selected)
{
    std::visit(
        [&selected, this](auto& data){
            if (data) {
                int dragPointId = 0;
                for (auto& country : data->countries) {
                    std::vector<ImVec2> points;
                    mapbox::geometry::polygon<double> polygon{mapbox::geometry::linear_ring<double>{}};
                    const auto color = computeColor(country.name);

                    points.reserve(country.borderContour.size());
                    for (auto& coordinate : country.borderContour) {
                        float size = POINT_SIZE;

                        if (selected && coordinate == *selected) {
                            size = SELECTED_POINT_SIZE;
                        }

                        const auto [x, y] = this->renderCoordinate(coordinate, color, size, dragPointId++);
                        polygon.back().emplace_back(x, y);
                        points.emplace_back(ImPlot::PlotToPixels(ImPlotPoint(x, y)));
                    }

                    if (country.borderContour.size() >= MINIMAL_POINTS_OF_POLYGON) {
                        const auto visualCenter = mapbox::polylabel(polygon, VISUAL_CENTER_PERCISION);
                        ImPlot::Annotation(visualCenter.x, visualCenter.y, color, COUNTRY_ANNOTATION_OFFSET, false, "%s", country.name.c_str());
                        ImPlot::SetNextFillStyle(color);
                        if (ImPlot::BeginItem(country.name.c_str(), ImPlotItemFlags_None, ImPlotCol_Fill)){
                            ImPlot::GetPlotDrawList()->AddConvexPolyFilled(points.data(), points.size(), IM_COL32(color.x * NORMALIZE, color.y * NORMALIZE, color.z * NORMALIZE, FILLED_ALPHA));
                            ImPlot::EndItem();
                        }
                    }
                }

                for (auto& city : data->cities) {
                    const auto color = computeColor(city.name);
                    float size = POINT_SIZE;

                    if (selected && city.coordinate == *selected) {
                        size = SELECTED_POINT_SIZE;
                    }

                    const auto [x, y] = this->renderCoordinate(city.coordinate, color, size, dragPointId++);
                    ImPlot::Annotation(x, y, color, CITY_ANNOTATION_OFFSET, true, "%s", city.name.c_str());
                }
            }
        },
        historicalData
    );    
}

tile::TileLoader& MapWidget::getTileLoader()
{
    return tileLoader;
}

}