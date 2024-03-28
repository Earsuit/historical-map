#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "src/tile/TileLoader.h"
#include "src/tile/Util.h"
#include "src/logger/Util.h"
#include "src/persistence/Data.h"
#include "src/ui/IInfoWidget.h"

#include "external/imgui/imgui.h"
#include "spdlog/spdlog.h"
#include "external/implot/implot.h"

#include <utility>
#include <memory>
#include <optional>

namespace ui {

constexpr auto MAP_WIDGET_NAME = "Map plot";

class MapWidget {
public:
    MapWidget(): 
        logger{spdlog::get(logger::LOGGER_NAME)}
    {}

    void paint(IInfoWidget& infoWidget);

    tile::TileLoader& getTileLoader();

private:
    // only compute the zoom level from level 0, otherwise the computed zoom level will be a mess
    static constexpr int BBOX_ZOOM_LEVEL = 0;

    std::shared_ptr<spdlog::logger> logger;
    tile::TileLoader tileLoader;

    std::pair<tile::BoundingBox, std::optional<ImPlotPoint>> renderMap(IInfoWidget& infoWidget, 
                                                                       ImVec2 size, 
                                                                       const std::string& name, 
                                                                       HistoricalData historicalData, 
                                                                       std::optional<persistence::Coordinate> selected);
    void renderOverlay(const std::string& name, int offset, const tile::BoundingBox& bbox, const std::optional<ImPlotPoint>& mousePos);
    void renderHistoricalInfo(HistoricalData historicalData, std::optional<persistence::Coordinate> selected);
    
    template<typename T>
    std::pair<double, double> renderCoordinate(T& coordinate, const ImVec4& color, float size, int id)
    {
        double x = tile::longitude2X(coordinate.longitude, BBOX_ZOOM_LEVEL);
        double y = tile::latitude2Y(coordinate.latitude, BBOX_ZOOM_LEVEL);
        
        if constexpr (std::is_const_v<T>) {
            ImPlot::DragPoint(id, &x, &y, color, size, ImPlotDragToolFlags_NoInputs);
        } else {
            if (ImPlot::DragPoint(id, &x, &y, color, size)) {
                coordinate.latitude = tile::y2Latitude(static_cast<float>(y), BBOX_ZOOM_LEVEL);
                coordinate.longitude = tile::x2Longitude(static_cast<float>(x), BBOX_ZOOM_LEVEL);
            }
        }

        logger->trace("Lon {}, lat {} at [{}, {}]",
                    coordinate.longitude,
                    coordinate.latitude,
                    x,
                    y);

        return {x, y};
    }
};
}

#endif
