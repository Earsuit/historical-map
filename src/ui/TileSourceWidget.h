#ifndef SRC_UI_TILE_SOURCE_WIDGET_H
#define SRC_UI_TILE_SOURCE_WIDGET_H

#include "src/ui/ITileSourceWidgetDetail.h"
#include "src/presentation/TileSourceWidgetPresenter.h"
#include "src/logger/ModuleLogger.h"

#include <string>
#include <functional>

namespace ui {
#define __(x) x     // gettext translation registration for constexpr

constexpr auto TILE_SOURCE_WIDGET_NAME = __("Tile source###TileSource");

class TileSourceWidget {
public:
    TileSourceWidget();

    void paint();

private:
    logger::ModuleLogger logger;
    presentation::TileSourceWidgetPresenter widgetPresenter;
    int sourceIdx = 0;
    int tileEngineIdx = 0;
    std::string sourceList;
    std::string engineListString;
    std::vector<std::string> engineList;
    std::vector<std::function<std::unique_ptr<ITileSourceWidgetDetail>()>> getDetailWidget;
    std::unique_ptr<ITileSourceWidgetDetail> detail;
};
}

#endif
