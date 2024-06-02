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

constexpr auto CITY_ANNOTATION_OFFSET = ImVec2(-15, 15);
constexpr auto COUNTRY_ANNOTATION_OFFSET = ImVec2(0, 0);

const auto ADD_NEW_COUNTRY_POPUP_NAME = "Add new country";
const auto ADD_NEW_CITY_POPUP_NAME = "Add new city";
constexpr float EPSILON = 1e-2;

bool operator==(const ImVec2& lhs, const ImVec2& rhs)
{
    return std::fabs(lhs.x - rhs.x) < EPSILON &&
           std::fabs(lhs.y - rhs.y) < EPSILON;
}

MapWidget::MapWidget(const std::string& source): 
    logger{spdlog::get(logger::LOGGER_NAME)},
    presenter{*this, source},
    source{source},
    plotName{"##" + source}
{
    util::signal::connect(&presenter,
                          &presentation::MapWidgetPresenter::countryUpdated,
                          this,
                          &MapWidget::onCountryUpdate);
    util::signal::connect(&presenter,
                          &presentation::MapWidgetPresenter::cityUpdated,
                          this,
                          &MapWidget::onCityUpdate);
    util::signal::connect(&presenter,
                          &presentation::MapWidgetPresenter::databaseCityListUpdated,
                          this,
                          &MapWidget::onDatabaseCityListUpdate);
}

MapWidget::~MapWidget()
{
    util::signal::disconnectAll(&presenter,
                                &presentation::MapWidgetPresenter::countryUpdated,
                                this);
    util::signal::disconnectAll(&presenter,
                                &presentation::MapWidgetPresenter::cityUpdated,
                                this);
    util::signal::disconnectAll(&presenter,
                                &presentation::MapWidgetPresenter::databaseCityListUpdated,
                                this);
}

void MapWidget::paint()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(0, 0));
    // we should render the map even if it is collapsed,
    // otherwise it will break when changing the docking layout
    ImGui::Begin(getName().c_str());
    ImGui::PopStyleVar(2);

    prepareRenderPoint();

    updatCountries();
    updateCities();

    renderMap();
    renderRightClickMenu();
    renderOverlay();

    ImGui::End();
}

void MapWidget::prepareRenderPoint()
{
    dragPointId = 0;
}

bool MapWidget::renderPoint(ImVec2& coordinate, float size, const ImVec4& color)
{
    double x = coordinate.x;
    double y = coordinate.y;
    if (ImPlot::DragPoint(dragPointId++, 
                          &x, 
                          &y, 
                          color, 
                          size)) {
        coordinate.x = static_cast<float>(x);
        coordinate.y = static_cast<float>(y);
        return true;
    }

    return false;
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
    if (ImPlot::BeginPlot(plotName.c_str(), ImGui::GetContentRegionAvail(), (ImPlotFlags_CanvasOnly ^ ImPlotFlags_NoLegend) | ImPlotFlags_Equal)) {
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
        renderCountries();
        renderCities();

        ImPlot::EndPlot();
    }
}

void MapWidget::renderRightClickMenu()
{
    bool openAddNewCountryPopup = false;
    bool openAddNewCityPopup = false;

    if (presenter.handleRequestHasRightClickMenu() && ImGui::BeginPopupContextItem(plotName.c_str(), ImGuiMouseButton_Right)) {
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && mousePos) {
            rightClickMenuPos = *mousePos;
            presenter.handleRequestCitiesFromDatabase();
        }

        if (ImGui::MenuItem("Add new country")) {
            openAddNewCountryPopup = true;
        }
        
        if (ImGui::BeginMenu("Add to exist country")) {
            const auto countries = presenter.handleRequestCountryList();
            for (const auto& country : countries) {
                if (ImGui::MenuItem(country.c_str())) {
                    presenter.handleExtendContour(country, 
                                                  model::Vec2{static_cast<float>(rightClickMenuPos.x), static_cast<float>(rightClickMenuPos.y)});
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Add new city")) {
            openAddNewCityPopup = true;
        }

        if (ImGui::BeginMenu("Add city from database")) {
            std::scoped_lock lk{lock};
            for (const auto& city : databaseCities) {
                if (ImGui::MenuItem(city.c_str())) {
                    presenter.handleAddCityFromDatabase(city);
                    break;
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    if (openAddNewCountryPopup) {
        ImGui::OpenPopup(ADD_NEW_COUNTRY_POPUP_NAME);
    }

    if (openAddNewCityPopup) {
        ImGui::OpenPopup(ADD_NEW_CITY_POPUP_NAME);
    }

    if (ImGui::BeginPopup(ADD_NEW_COUNTRY_POPUP_NAME)) {
        ImGui::InputTextWithHint("##", "Country Name", &newCountryName);
        if (ImGui::Button("Add") & !newCountryName.empty()) {
            if (presenter.handleAddCountry(newCountryName, 
                                           model::Vec2{static_cast<float>(rightClickMenuPos.x), static_cast<float>(rightClickMenuPos.y)})) {
                newCountryName.clear();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup(ADD_NEW_CITY_POPUP_NAME)) {
        ImGui::InputTextWithHint("##", "City Name", &newCityName);
        if (ImGui::Button("Add") & !newCityName.empty()) {
            if (presenter.handleAddCity(newCityName, 
                                        model::Vec2{static_cast<float>(rightClickMenuPos.x), static_cast<float>(rightClickMenuPos.y)})) {
                newCountryName.clear();
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}

void MapWidget::renderOverlay()
{
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | 
                                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 work_pos = ImGui::GetWindowPos(); // Use work area to avoid menu-bar/task-bar, if any!
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

void MapWidget::updatCountries()
{
    if (countryUpdated) {
        countryUpdated = false;

        countries.clear();

        for (const auto& name : presenter.handleRequestCountryList()) {
            mapbox::geometry::polygon<double> polygon{mapbox::geometry::linear_ring<double>{}};
            Country country{presenter.handleRequestColor(name)};

            auto contour = presenter.handleRequestContour(name);

            for (const auto& coord : contour) {
                country.contour.emplace_back(coord);
                polygon.back().emplace_back(coord.x, coord.y);
            }

            const auto visualCenter = mapbox::polylabel<double>(polygon, VISUAL_CENTER_PERCISION);
            country.labelCoordinate = ImVec2{static_cast<float>(visualCenter.x), static_cast<float>(visualCenter.y)};

            countries.emplace(std::make_pair(name, country));
        }
    }
}

void MapWidget::updateCities()
{
    if (cityUpdated) {
        cityUpdated = false;

        cities.clear();

        for (const auto& city : presenter.handleRequestCityList()) {
            if (const auto coord = presenter.handleRequestCityCoord(city); coord) {
                cities.emplace(std::make_pair(city, City{presenter.handleRequestColor(city), *coord}));
            }
        }
    }
}

void MapWidget::renderCountries()
{
    for (auto& [name, country] : countries) {
        int idx = 0;
        std::vector<ImVec2> pixels;
        pixels.reserve(country.contour.size());

        for (auto& coord : country.contour) {
            const auto size = presenter.handleRequestCoordSize(coord);
            if (renderPoint(coord, size, country.color)) {
                presenter.handleUpdateContour(name, idx, coord);
            }

            pixels.emplace_back(ImPlot::PlotToPixels(ImPlotPoint(coord.x, coord.y)));

            idx++;
        }

        if (country.contour.size() >= MINIMAL_POINTS_OF_POLYGON) {
            ImPlot::Annotation(country.labelCoordinate.x, 
                               country.labelCoordinate.y, 
                               country.color, 
                               COUNTRY_ANNOTATION_OFFSET, 
                               false, 
                               "%s", 
                               name.c_str());

            ImPlot::SetNextFillStyle(country.color);
            if (ImPlot::BeginItem(name.c_str(), ImPlotItemFlags_None, ImPlotCol_Fill)) {
                ImPlot::GetPlotDrawList()->AddConvexPolyFilled(pixels.data(), 
                                                               pixels.size(), 
                                                               IM_COL32(country.color.x * NORMALIZE, 
                                                                        country.color.y * NORMALIZE, 
                                                                        country.color.z * NORMALIZE,
                                                                        FILLED_ALPHA));
                ImPlot::EndItem();
            }
        }
    }
}

void MapWidget::renderCities()
{
    for (auto& [name, city] : cities) {
        const auto size = presenter.handleRequestCoordSize(city.coordinate);
        if (renderPoint(city.coordinate, size, city.color)) {
            presenter.handleUpdateCity(name, city.coordinate);
        }

        ImPlot::Annotation(city.coordinate.x, 
                           city.coordinate.y, 
                           city.color, 
                           CITY_ANNOTATION_OFFSET, 
                           false, 
                           "%s", 
                           name.c_str());
    }
}

void MapWidget::onDatabaseCityListUpdate(std::vector<std::string>&& cities)
{
    std::scoped_lock lk{lock};
    databaseCities = std::move(cities);
}
}