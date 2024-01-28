#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "src/tile/TileLoader.h"
#include "src/tile/Util.h"
#include "src/logger/Util.h"
#include "src/persistence/Data.h"

#include "external/imgui/imgui.h"
#include "spdlog/spdlog.h"
#include "external/implot/implot.h"

#include <utility>
#include <memory>

namespace ui {

constexpr auto MAP_WIDGET_NAME = "Map plot";

class MapWidget {
public:
    MapWidget(): logger{spdlog::get(logger::LOGGER_NAME)} {}

    void paint(std::pair<std::shared_ptr<persistence::Data>, std::optional<persistence::Coordinate>> info);

    tile::TileLoader& getTileLoader();

private:
    std::shared_ptr<spdlog::logger> logger;
    tile::TileLoader tileLoader;
    int zoom = 0;
    tile::BoundingBox bbox;
    ImPlotPoint mousePos = {0.0f, 0.0f};

    void renderTile(std::pair<std::shared_ptr<persistence::Data>, std::optional<persistence::Coordinate>> info);
    void renderOverlay();
    void renderHistoricalInfo(std::pair<std::shared_ptr<persistence::Data>, std::optional<persistence::Coordinate>> info);
    ImVec2 renderCoordinate(persistence::Coordinate& coordinate, const ImVec4& color, float size, int id);
};
}

#endif
