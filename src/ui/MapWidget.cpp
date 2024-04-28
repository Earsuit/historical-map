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
    const auto infos = infoWidget.getInfos();

    const ImVec2 singleMapWindowSize = {area.x / infos.size(), area.y};
    int overlayOffset = 0;
    const auto hovered = infoWidget.getHovered();

    for (auto [info, name] : infos) {
        const auto mousePos = renderMap(infoWidget, singleMapWindowSize, name, info, hovered);
        renderOverlay(name, overlayOffset, mousePos);
        overlayOffset += singleMapWindowSize.x + ImPlot::GetStyle().PlotPadding.x - ImPlot::GetStyle().PlotBorderSize * 2;
        ImGui::SameLine();
    }

    ImGui::End();
}

std::optional<ImPlotPoint> MapWidget::renderMap(IInfoWidget& infoWidget,
                                                ImVec2 size, 
                                                const std::string& name, 
                                                HistoricalInfo historicalInfo, 
                                                std::optional<persistence::Coordinate> hovered)
{
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

        logger->trace("Plot limit X [{}, {}], Y [{}, {}]", plotLimits.X.Min, plotLimits.X.Max, plotLimits.Y.Min, plotLimits.Y.Max);
        logger->trace("Plot size x={}, y={} pixels", plotSize.x, plotSize.y);

        const auto tiles = presenter.getTiles({static_cast<float>(plotLimits.X.Min), static_cast<float>(plotLimits.X.Max)},
                                              {static_cast<float>(plotLimits.Y.Min), static_cast<float>(plotLimits.Y.Max)},
                                              {plotSize.x, plotSize.y});

        for (const auto tile : tiles) {
            const auto coord = tile->getCoordinate();
            const ImVec2 bMax = {presenter.computeTileBound(coord.x+1), presenter.computeTileBound(coord.y+1)};
            const ImVec2 bMin = {presenter.computeTileBound(coord.x), presenter.computeTileBound(coord.y)};
            ImPlot::PlotImage("##", tile->getTexture(), bMin, bMax);
        }

        renderHistoricalInfo(historicalInfo, hovered);

        ImPlot::EndPlot();
    }

    if (ImGui::BeginPopupContextItem(plotName.c_str())) {
        static float longitue = 0, latitude = 0;
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && mousePos) {
            longitue = presenter.x2Longitude(mousePos->x);
            latitude = presenter.y2Latitude(mousePos->y);
        }

        infoWidget.drawRightClickMenu(longitue, latitude);
        ImGui::EndPopup();
    }

    return mousePos;
}

void MapWidget::renderOverlay(const std::string& name, int offset, const std::optional<ImPlotPoint>& mousePos)
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
                        presenter.x2Longitude(mousePos->x), 
                        presenter.y2Latitude(mousePos->y));
        } else {
            ImGui::Text("Cursor at: ");
        }

        const auto bbox = presenter.getBoundingBox();

        ImGui::Text("View range: west %.2f, east %.2f, \n\t\t\tnorth %.2f, south %.2f", bbox.west, bbox.east, bbox.north, bbox.south);
    }
    ImGui::PopStyleColor(2);
    ImGui::End();
}

void MapWidget::renderHistoricalInfo(HistoricalInfo historicalInfo, std::optional<persistence::Coordinate> hovered)
{
    std::visit(
        [&hovered, this](auto& data){
            if (data) {
                int dragPointId = 0;
                for (auto& country : data->countries) {
                    std::vector<ImVec2> points;
                    mapbox::geometry::polygon<double> polygon{mapbox::geometry::linear_ring<double>{}};
                    const auto color = computeColor(country.name);

                    points.reserve(country.borderContour.size());
                    for (auto& coordinate : country.borderContour) {
                        float size = POINT_SIZE;

                        if (hovered && coordinate == *hovered) {
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

                    if (hovered && city.coordinate == *hovered) {
                        size = SELECTED_POINT_SIZE;
                    }

                    const auto [x, y] = this->renderCoordinate(city.coordinate, color, size, dragPointId++);
                    ImPlot::Annotation(x, y, color, CITY_ANNOTATION_OFFSET, true, "%s", city.name.c_str());
                }
            }
        },
        historicalInfo
    );    
}

}