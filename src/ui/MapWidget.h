#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "src/tile/TileLoader.h"
#include "src/tile/Util.h"
#include "src/logger/Util.h"
#include "src/persistence/Data.h"
#include "src/ui/HistoricalInfoWidget.h"

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
    MapWidget(HistoricalInfoWidget& historicalInfoWidget): 
        logger{spdlog::get(logger::LOGGER_NAME)},
        historicalInfoWidget{historicalInfoWidget}
    {}

    void paint();

    tile::TileLoader& getTileLoader();

private:
    std::shared_ptr<spdlog::logger> logger;
    HistoricalInfoWidget& historicalInfoWidget;
    tile::TileLoader tileLoader;

    std::pair<tile::BoundingBox, std::optional<ImPlotPoint>> renderMap(ImVec2 size, const std::string& name, std::shared_ptr<persistence::Data> info, std::optional<persistence::Coordinate> selected);
    void renderOverlay(const std::string& name, int offset, const tile::BoundingBox& bbox, const std::optional<ImPlotPoint>& mousePos);
    void renderHistoricalInfo(std::shared_ptr<persistence::Data> info, std::optional<persistence::Coordinate> selected);
    std::pair<double, double> renderCoordinate(persistence::Coordinate& coordinate, const ImVec4& color, float size, int id);
};
}

#endif
