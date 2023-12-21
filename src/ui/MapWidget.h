#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "src/tile/TileSource.h"
#include "src/tile/TileLoader.h"
#include "src/logger/Util.h"

#include "external/imgui/imgui.h"
#include "spdlog/spdlog.h"

#include <utility>
#include <memory>

namespace ui {

class MapWidget {
public:
    MapWidget(): logger{spdlog::get(logger::LOGGER_NAME)} {}
    void setTileSource(std::shared_ptr<tile::TileSource> tileSource);
    void paint();

private:
    std::shared_ptr<spdlog::logger> logger;
    tile::TileLoader tileLoader;
    int zoom = 0;   

    std::pair<ImVec2, ImVec2> calculateBound(int x, int y);
};
}

#endif
