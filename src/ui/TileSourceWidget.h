#ifndef SRC_UI_TILE_SOURCE_WIDGET_H
#define SRC_UI_TILE_SOURCE_WIDGET_H

#include "src/tile/TileSource.h"

#include <string>
#include <memory>
#include <functional>

namespace ui {
class TileSourceWidget {
public:
    TileSourceWidget();

    void paint();
    std::shared_ptr<tile::TileSource> getTileSource();

private:
    int sourceIdx = 0;
    std::shared_ptr<tile::TileSource> tileSource;
    std::vector<std::function<void()>> showConfigWidget;

    void showTileSourceUrlConfig();
};
}

#endif
