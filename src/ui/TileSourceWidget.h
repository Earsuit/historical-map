#ifndef SRC_UI_TILE_SOURCE_WIDGET_H
#define SRC_UI_TILE_SOURCE_WIDGET_H

#include "src/tile/TileSource.h"
#include "src/tile/TileEngine.h"

#include "spdlog/spdlog.h"

#include <string>
#include <memory>
#include <functional>

namespace ui {
constexpr auto TILE_SOURCE_WIDGET_NAME = "Tile source";

class TileSourceWidget {
public:
    TileSourceWidget();

    void paint();
    std::shared_ptr<tile::TileSource> getTileSource();
    std::shared_ptr<tile::TileEngine> getTileEngine();

private:
    std::shared_ptr<spdlog::logger> logger;
    int sourceIdx = 0;
    int tileEngineIdx = 0;
    std::shared_ptr<tile::TileSource> tileSource;
    std::shared_ptr<tile::TileEngine> tileEngine;
    std::vector<std::function<void()>> showConfigWidget;

    void showTileSourceUrlConfig();
};
}

#endif
