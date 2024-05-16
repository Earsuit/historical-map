#include "src/presentation/MapWidgetPresenter.h"
#include "src/presentation/Util.h"
#include "src/logger/Util.h"

#include "mapbox/polylabel.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>

namespace presentation {
constexpr int TILE_SIZE = 256;  
constexpr int BBOX_ZOOM_LEVEL = 0;
constexpr int MIN_ZOOM_LEVEL = 0;
constexpr int MAX_ZOOM_LEVEL = 18;
constexpr float PI_DEG = 360.0;
constexpr float HALF_PI_DEG = 180.0;
constexpr int PADDING = 0;
constexpr float MAX_LONGITUDE = 180.0f;
constexpr float MIN_LONGITUDE = -180.0f;
constexpr float MAX_LATITUDE = 85.05112878f;
constexpr float MIN_LATITUDE = -85.05112878f;
constexpr float PI = M_PI;
constexpr auto MINIMAL_POINTS_OF_POLYGON = 3;
constexpr auto VISUAL_CENTER_PERCISION = 1.0;
constexpr float POINT_SIZE = 2.0f;
constexpr float HOVERED_POINT_SIZE = 4.0f;
constexpr auto DEFAULT_ALPHA = 1.0f;
constexpr auto NORMALIZE = 255.0f; 
constexpr uint8_t MASK = 0xFF;
constexpr auto TEXT_DECIMAL_PRECISION = 2;

Color computeColor(const std::string& val)
{
    const auto hash = std::hash<std::string>{}(val);

    float r = static_cast<float>((hash & MASK) / NORMALIZE);
    float g = static_cast<float>(((hash >> 8) & MASK) / NORMALIZE);
    float b = static_cast<float>(((hash >> 16) & MASK) / NORMALIZE);

    return Color(r, g, b, DEFAULT_ALPHA);
}

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
    tileModel{model::TileModel::getInstance()},
    dynamicInfoModel{model::DynamicInfoModel::getInstance()},
    source{source}
{
    startWorkerThread();
}

MapWidgetPresenter::~MapWidgetPresenter()
{
    stopWorkerThread();
}

void MapWidgetPresenter::startWorkerThread()
{
    runWorkerThread = true;
    workerThread = std::thread(&MapWidgetPresenter::worker, this);
}

void MapWidgetPresenter::stopWorkerThread()
{
    runWorkerThread = false;

    // enqueue an empty task to wake up the wait_dequeue if necessary
    taskQueue.enqueue([](){});

    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void MapWidgetPresenter::worker()
{
    while (runWorkerThread) {
        std::function<void()> task; 

        taskQueue.wait_dequeue(task);

        task();
    }
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

void MapWidgetPresenter::handleRenderCountry()
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        for (const auto& name : info->getCountryList()) {
            // have to use double type here due to a compilation error in mapbox::polylabel if use float
            mapbox::geometry::polygon<double> polygon{mapbox::geometry::linear_ring<double>{}};
            std::vector<model::Vec2> points;
            const auto color = computeColor(name);
            auto& country = info->getCountry(name);

            for (auto& coordinate : country.borderContour) {
                const auto point = handleRenderCoordinate(coordinate, color);
                polygon.back().emplace_back(point.x, point.y);
                points.emplace_back(point);
            }

            if (points.size() >= MINIMAL_POINTS_OF_POLYGON) {
                const auto visualCenter = mapbox::polylabel<double>(polygon, VISUAL_CENTER_PERCISION);
                view.renderAnnotation(model::Vec2{static_cast<float>(visualCenter.x), static_cast<float>(visualCenter.y)}, 
                                    country.name, 
                                    color);
                view.renderContour(country.name, points, color);
            }
        }
    }
}

void MapWidgetPresenter::handleRenderCity()
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        for (const auto& name : info->getCityList()) {
            const auto color = computeColor(name);
            auto& city = info->getCity(name);
            const auto point = handleRenderCoordinate(city.coordinate, color);
            view.renderAnnotation(point, city.name, color);
        }
    }
}

model::Vec2 MapWidgetPresenter::handleRenderCoordinate(persistence::Coordinate& coordinate,
                                                                       const Color& color)
{
    bool changed = false;
    const auto hovered = dynamicInfoModel.getHoveredCoord();
    const model::Vec2 point{model::longitude2X(coordinate.longitude, BBOX_ZOOM_LEVEL), 
                            model::latitude2Y(coordinate.latitude, BBOX_ZOOM_LEVEL)};

    float size = POINT_SIZE;
    if (hovered && coordinate == *hovered) {
        size = HOVERED_POINT_SIZE;
    }

    const auto newPoint = view.renderPoint(point, size, color);

    if (newPoint != point) {
        coordinate.latitude = model::y2Latitude(point.x, BBOX_ZOOM_LEVEL);
        coordinate.longitude = model::x2Longitude(point.y, BBOX_ZOOM_LEVEL);
    }

    return newPoint;
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
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        return info->getCountryList();
    }

    return {};
}

void MapWidgetPresenter::handleExtendContour(const std::string& name, const model::Vec2& pos)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        if (info->containsCountry(name)) {
            auto& country = info->getCountry(name);
            auto longitude = x2Longitude(pos.x);
            auto latitude = y2Latitude(pos.y);
            country.borderContour.emplace_back(persistence::Coordinate{latitude, longitude});
        } else {
            logger->error("Extend contour fail because country {} doesn't exist from source {}", name, source);
        }
    } else {
        logger->error("Extend contour fail because historical info is null from source {}", source);
    }
}

bool MapWidgetPresenter::handleAddCountry(const std::string& name, const model::Vec2& pos)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        auto longitude = x2Longitude(pos.x);
        auto latitude = y2Latitude(pos.y);
        if (!info->addCountry(persistence::Country{name, {persistence::Coordinate{latitude, longitude}}})) {
            logger->error("Add country {} fail because it already exists from source {}", name, source);
            return false;
        }
    } else {
        logger->error("Add country {} fail because historical info is null from source {}", name, source);
        return false;
    }
    return true;
}

bool MapWidgetPresenter::handleAddCity(const std::string& name, const model::Vec2& pos)
{
    if (auto info = dynamicInfoModel.getHistoricalInfo(source); info) {
        auto longitude = x2Longitude(pos.x);
        auto latitude = y2Latitude(pos.y);
        if (!info->addCity(name, persistence::Coordinate{latitude, longitude})) {
            logger->error("Add city {} fail because it already exists from source {}", name, source);
            return false;
        }
    } else {
        logger->error("Add city {} fail because historical info is null from source {}", name, source);
        return false;
    }

    return true;
}
}