#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "src/tile/TileSource.h"
#include "src/tile/TileLoader.h"
#include "src/tile/Util.h"
#include "src/tile/TileEngine.h"
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
    void setTileSource(std::shared_ptr<tile::TileSource> tileSource);
    void setTileEngine(std::shared_ptr<tile::TileEngine> tileEngine);
    void paint(std::shared_ptr<persistence::Data> info);

private:
    std::shared_ptr<spdlog::logger> logger;
    tile::TileLoader tileLoader;
    int zoom = 0;
    tile::BoundingBox bbox;
    ImPlotPoint mousePos = {0.0f, 0.0f};

    void renderTile(std::shared_ptr<persistence::Data> info);
    void overlay();
    void renderHistoricalInfo(std::shared_ptr<persistence::Data> info);
};
}

#endif
