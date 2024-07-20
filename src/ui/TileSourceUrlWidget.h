#ifndef SRC_UI_TILE_SOURCE_URL_WIDGET_H
#define SRC_UI_TILE_SOURCE_URL_WIDGET_H

#include "src/ui/ITileSourceWidgetDetail.h"
#include "src/presentation/TileSourceUrlPresenter.h"

#include <string>

namespace ui {
class TileSourceUrlWidget: public ITileSourceWidgetDetail {
public:
    TileSourceUrlWidget();
    ~TileSourceUrlWidget() override = default;

    void paint() override;

private:
    std::string url;
    presentation::TileSourceUrlPresenter presenter;
};
}

#endif /* SRC_UI_TILE_SOURCE_URL_WIDGET_H */
