#include "src/presentation/MapWidgetPresenter.h"
#include "src/presentation/Util.h"
#include "src/logger/Util.h"

#include <algorithm>
#include <sstream>
#include <iomanip>

namespace presentation {
constexpr int BBOX_ZOOM_LEVEL = 0;
constexpr int MIN_ZOOM_LEVEL = 0;
constexpr float MAX_LONGITUDE = 180.0f;
constexpr float MIN_LONGITUDE = -180.0f;
constexpr float MAX_LATITUDE = 85.05112878f;
constexpr float MIN_LATITUDE = -85.05112878f;
constexpr float POINT_SIZE = 2.0f;
constexpr float HOVERED_POINT_SIZE = 4.0f;
constexpr auto DEFAULT_ALPHA = 1.0f;
constexpr auto NORMALIZE = 255.0f; 
constexpr uint8_t MASK = 0xFF;
constexpr auto TEXT_DECIMAL_PRECISION = 2;

float x2Longitude(float x)
{
    return std::clamp(model::x2Longitude(x, BBOX_ZOOM_LEVEL), MIN_LONGITUDE, MAX_LONGITUDE);
}

float y2Latitude(float y)
{
    return std::clamp(model::y2Latitude(y, BBOX_ZOOM_LEVEL), MIN_LATITUDE, MAX_LATITUDE);
}

MapWidgetPresenter::MapWidgetPresenter(MapWidgetInterface& view, const std::string& source):
    logger{spdlog::get(logger::LOGGER_NAME)},
    view{view},
    databaseModel{model::DatabaseModel::getInstance()},
    tileModel{model::TileModel::getInstance()},
    dynamicInfoModel{model::DynamicInfoModel::getInstance()},
    source{source}
{
    util::signal::connect(&dynamicInfoModel,
                          &model::DynamicInfoModel::onCountryUpdate,
                          this,
                          &MapWidgetPresenter::onCountryUpdate);
    util::signal::connect(&dynamicInfoModel,
                          &model::DynamicInfoModel::onCityUpdate,
                          this,
                          &MapWidgetPresenter::onCityUpdate);
    util::signal::connect(&databaseModel,
                          &model::DatabaseModel::onYearChange,
                          this,
                          &MapWidgetPresenter::onYearChange);

    year = databaseModel.getYear();
}

MapWidgetPresenter::~MapWidgetPresenter()
{
    util::signal::disconnectAll(&dynamicInfoModel,
                                &model::DynamicInfoModel::onCountryUpdate,
                                this);
    util::signal::disconnectAll(&dynamicInfoModel,
                                &model::DynamicInfoModel::onCityUpdate,
                                this);
    util::signal::disconnectAll(&databaseModel,
                                &model::DatabaseModel::onYearChange,
                                this);
}

void MapWidgetPresenter::handleRenderTiles()
{
    const auto xAxis = view.getAxisRangeX();
    const auto yAxis = view.getAxisRangeY();
    const auto plotSize = view.getPlotSize();
    const auto tiles = tileModel.getTiles(xAxis, yAxis, plotSize);

    for (const auto tile : tiles) {
        const auto coord = tile->getCoordinate();
        const auto bMax = tileModel.getTileBoundMax(tile);
        const auto bMin = tileModel.getTileBoundMin(tile);
        view.renderTile(tile->getTexture(), bMin, bMax);
    }
}

std::string MapWidgetPresenter::handleGetOverlayText() const
{
    std::stringstream text;
    const auto bbox = tileModel.getBoundingBox();

    text << std::fixed << std::setprecision(TEXT_DECIMAL_PRECISION);

    text << "Cursor at: ";
    if (auto mouse = view.getMousePos(); mouse) {
        text << "lon " 
             << x2Longitude(mouse->x)
             << ", lat " 
             << y2Latitude(mouse->y);
    }
    text << "\n"
         << "View range: west "
         << bbox.west
         << ", east "
         << bbox.east
         << ", \n\t\t\tnorth "
         << bbox.north
         << ", south "
         << bbox.south;

    return text.str();
}

bool MapWidgetPresenter::handleRequestHasRightClickMenu() const noexcept
{
    if (source == DEFAULT_HISTORICAL_INFO_SOURCE) {
        return true;
    }
    
    return false;
}

std::vector<std::string> MapWidgetPresenter::handleRequestCountryList() const
{
    return dynamicInfoModel.getCountryList(source, year);
}

std::vector<std::string> MapWidgetPresenter::handleRequestCityList() const
{
    return dynamicInfoModel.getCityList(source, year);
}

std::vector<ImVec2> MapWidgetPresenter::handleRequestContour(const std::string& name) const
{
    const auto contour = dynamicInfoModel.getContour(source, year, name);
    std::vector<ImVec2> points;
    points.reserve(contour.size());
    for (const auto& coord : contour) {
        points.emplace_back(model::longitude2X(coord.longitude, BBOX_ZOOM_LEVEL),
                            model::latitude2Y(coord.latitude, BBOX_ZOOM_LEVEL));
    }

    return points;
}

ImVec4 MapWidgetPresenter::handleRequestColor(const std::string& name) const
{
    const auto hash = std::hash<std::string>{}(name);

    float r = static_cast<float>((hash & MASK) / NORMALIZE);
    float g = static_cast<float>(((hash >> 8) & MASK) / NORMALIZE);
    float b = static_cast<float>(((hash >> 16) & MASK) / NORMALIZE);

    return ImVec4(r, g, b, DEFAULT_ALPHA);
}

std::optional<ImVec2> MapWidgetPresenter::handleRequestCityCoord(const std::string& name) const
{
    if (const auto& city = dynamicInfoModel.getCity(source, year, name); city) {
        return ImVec2{model::longitude2X(city->coordinate.longitude, BBOX_ZOOM_LEVEL), 
                      model::latitude2Y(city->coordinate.latitude, BBOX_ZOOM_LEVEL)};
    }

    return std::nullopt;
}

float MapWidgetPresenter::handleRequestCoordSize(const ImVec2& coord) const
{
    if (const auto hovered = dynamicInfoModel.getHoveredCoord(); hovered) {
        const auto point = ImVec2{model::longitude2X(hovered->longitude, BBOX_ZOOM_LEVEL), 
                                  model::latitude2Y(hovered->latitude, BBOX_ZOOM_LEVEL)};
        if (point.x == coord.x && point.y == coord.y) {
            return HOVERED_POINT_SIZE;
        }
    }

    return POINT_SIZE;
}

void MapWidgetPresenter::handleUpdateContour(const std::string& name, int idx, const ImVec2& coord)
{
    dynamicInfoModel.updateContour(source, 
                                   year, 
                                   name, 
                                   idx, 
                                   persistence::Coordinate{y2Latitude(coord.y), x2Longitude(coord.x)});
}

void MapWidgetPresenter::handleUpdateCity(const std::string& name, const ImVec2& coord)
{
    dynamicInfoModel.updateCityCoord(source, 
                                     year, 
                                     name, 
                                     persistence::Coordinate{y2Latitude(coord.y), x2Longitude(coord.x)});
}

bool MapWidgetPresenter::handleExtendContour(const std::string& name, const model::Vec2& pos)
{
    const persistence::Coordinate coord{.latitude = y2Latitude(pos.y), .longitude = x2Longitude(pos.x)};

    if (dynamicInfoModel.extendContour(source, year, name, coord)) {
        return true;
    }

    logger->error("Extend country {} contour fail for source", name, source);
    return false;
}

bool MapWidgetPresenter::handleAddCountry(const std::string& name, const model::Vec2& pos)
{
    const persistence::Country country{name, {persistence::Coordinate{.latitude = y2Latitude(pos.y), .longitude = x2Longitude(pos.x)}}};
    if (dynamicInfoModel.addCountry(source, year, country)) {
        return true;
    }

    logger->error("Add country {} fail for source {}", name, source);

    return false;
}

bool MapWidgetPresenter::handleAddCity(const std::string& name, const model::Vec2& pos)
{
    const persistence::City city{name, {.latitude = y2Latitude(pos.y), .longitude = x2Longitude(pos.x)}};

    if (dynamicInfoModel.addCity(source, year, city)) {
        return true;
    }

    logger->error("Add city {} fail for source {}", name, source);

    return false;
}

bool MapWidgetPresenter::varifySignal(const std::string& source, int year) const noexcept
{
    return source == this->source && year == this->year;
}

void MapWidgetPresenter::onCountryUpdate(const std::string& source, int year)
{
    if (varifySignal(source, year)) {
        logger->debug("MapWidgetPresenter onCountryUpdate for source {} at year {}", source, year);
        countryUpdated();
    }
}

void MapWidgetPresenter::onCityUpdate(const std::string& source, int year)
{
    if (varifySignal(source, year)) {
        logger->debug("MapWidgetPresenter onCityUpdate for source {} at year {}", source, year);
        cityUpdated();
    }
}

void MapWidgetPresenter::onYearChange(int year)
{
    this->year = year;
    countryUpdated();
    cityUpdated();
}
}