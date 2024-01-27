#ifndef SRC_UI_TILE_SOURCE_WIDGET_H
#define SRC_UI_TILE_SOURCE_WIDGET_H

#include "src/tile/TileLoader.h"

#include "spdlog/spdlog.h"

#include <string>
#include <memory>
#include <functional>

namespace ui {
constexpr auto TILE_SOURCE_WIDGET_NAME = "Tile source";

class TileSourceWidget {
public:
    TileSourceWidget(tile::TileLoader& tileLoader);

    void paint();

private:
    std::shared_ptr<spdlog::logger> logger;
    tile::TileLoader& tileLoader;
    int sourceIdx = 0;
    int tileEngineIdx = 0;
    std::vector<std::function<void()>> showConfigWidget;

    void showTileSourceUrlConfig();
};
}

#endif
