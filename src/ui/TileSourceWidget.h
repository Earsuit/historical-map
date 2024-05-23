#ifndef SRC_UI_TILE_SOURCE_WIDGET_H
#define SRC_UI_TILE_SOURCE_WIDGET_H

#include "src/presentation/TileSourceWidgetPresenter.h"

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

private:
    std::shared_ptr<spdlog::logger> logger;
    presentation::TileSourceWidgetPresenter widgetPresenter;
    int sourceIdx = 0;
    int tileEngineIdx = 0;
    std::string sourceList;
    std::string engineListString;
    std::vector<std::string> engineList;
    std::vector<std::function<void()>> showConfigWidget;

    void showTileSourceUrlConfig();
};
}

#endif
