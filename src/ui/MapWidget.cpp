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

void MapWidget::paint()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(0, 0));
    if (ImGui::Begin(MAP_WIDGET_NAME)) {
        ImGui::PopStyleVar(2);

        renderMap();
        renderOverlay();

        ImGui::End();
    } 
}

void MapWidget::renderAnnotation(const model::Vec2& coordinate, const std::string& name, const presentation::Color& color)
{
    ImPlot::Annotation(static_cast<double>(coordinate.x), static_cast<double>(coordinate.y), 
                       ImVec4{color.red, color.green, color.blue, color.alpha}, 
                       COUNTRY_ANNOTATION_OFFSET, 
                       false, 
                       "%s", 
                       name.c_str());
}

model::Vec2 MapWidget::renderPoint(const model::Vec2& coordinate, float size, const presentation::Color& color)
{
    double x = coordinate.x;
    double y = coordinate.y;
    ImPlot::DragPoint(dragPointId++, &x, &y, ImVec4{color.red, color.green, color.blue, color.alpha}, size);

    return {static_cast<float>(x), static_cast<float>(y)};
}

void MapWidget::renderContour(const std::string& name, const std::vector<model::Vec2>& contour, const presentation::Color& color)
{
    ImPlot::SetNextFillStyle(ImVec4{color.red, color.green, color.blue, color.alpha});
    if (ImPlot::BeginItem(name.c_str(), ImPlotItemFlags_None, ImPlotCol_Fill)){
        std::vector<ImVec2> points;
        points.reserve(contour.size());

        for (const auto& [x, y] : contour) {
            points.emplace_back(ImPlot::PlotToPixels(ImPlotPoint(x, y)));
        }

        ImPlot::GetPlotDrawList()->AddConvexPolyFilled(points.data(), 
                                                       points.size(), 
                                                       IM_COL32(color.red * NORMALIZE, color.green * NORMALIZE, color.blue * NORMALIZE, FILLED_ALPHA));
        ImPlot::EndItem();
    }
}

model::Range MapWidget::getAxisRangeX() const noexcept
{
    return {static_cast<float>(plotRect.X.Min), static_cast<float>(plotRect.X.Max)};
}

model::Range MapWidget::getAxisRangeY() const noexcept
{
    return {static_cast<float>(plotRect.Y.Min), static_cast<float>(plotRect.Y.Max)};
}

model::Vec2 MapWidget::getPlotSize() const noexcept
{
    return {plotSize.x, plotSize.y};
}

void MapWidget::renderTile(void* texture, const model::Vec2& bMin, const model::Vec2& bMax)
{
    ImPlot::PlotImage("##", texture, ImVec2{bMin.x, bMin.y}, ImVec2{bMax.x, bMax.y});
}

std::optional<model::Vec2> MapWidget::getMousePos() const
{
    if (mousePos) {
        return model::Vec2{static_cast<float>(mousePos->x), static_cast<float>(mousePos->y)};
    } 

    return std::nullopt;
}

void MapWidget::renderMap()
{
    if (ImPlot::BeginPlot("##", ImGui::GetContentRegionAvail(), (ImPlotFlags_CanvasOnly ^ ImPlotFlags_NoLegend) | ImPlotFlags_Equal)) {
        ImPlot::SetupLegend(ImPlotLocation_NorthEast, ImPlotLegendFlags_NoButtons);
        ImPlot::SetupAxis(ImAxis_X1, nullptr, AXIS_FLAGS);
        ImPlot::SetupAxis(ImAxis_Y1, nullptr, AXIS_FLAGS | ImPlotAxisFlags_Invert);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, MIN_AXIS_RANGE, MAX_AXIS_RANGE);
        ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, MIN_AXIS_RANGE, MAX_AXIS_RANGE);
        ImPlot::SetupAxisLimits(ImAxis_X1, INIT_X_LIMIT_MIN, INIT_X_LIMIT_MAX);
        ImPlot::SetupAxisLimits(ImAxis_Y1, INIT_Y_LIMIT_MIN, INIT_Y_LIMIT_MAX);
        
        plotSize = ImPlot::GetPlotSize();
        plotRect = ImPlot::GetPlotLimits(ImAxis_X1, ImAxis_Y1);
        if (const auto cursor = ImPlot::GetPlotMousePos(); 
            inBound(cursor.x, plotRect.X.Min, plotRect.X.Max) &&
            inBound(cursor.y, plotRect.Y.Min, plotRect.Y.Max)) {
            mousePos = cursor;
        } else {
            mousePos = std::nullopt;
        }

        logger->trace("Plot limit X [{}, {}], Y [{}, {}]", plotRect.X.Min, plotRect.X.Max, plotRect.Y.Min, plotRect.Y.Max);
        logger->trace("Plot size x={}, y={} pixels", plotSize.x, plotSize.y);

        presenter.handleRenderTiles();
        presenter.handleRenderCountry();
        presenter.handleRenderCity();

        ImPlot::EndPlot();
    }
}

void MapWidget::renderOverlay()
{
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | 
                                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 window_pos;
    window_pos.x = work_pos.x + OVERLAY_PAD;
    window_pos.y = work_pos.y + OVERLAY_PAD;
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleColor(ImGuiCol_TitleBg, OVERLAY_BACKGROUND_COLOR);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, OVERLAY_BACKGROUND_COLOR);
    if (ImGui::Begin((source + "##Overlay").c_str(), nullptr, windowFlags)) {
        ImGui::Text("%s", presenter.handleGetOverlayText().c_str());
    }
    ImGui::PopStyleColor(2);
    ImGui::End();
}
}