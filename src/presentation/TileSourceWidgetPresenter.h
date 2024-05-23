#ifndef SRC_PRESENTATION_TILE_SOURCE_WIDGET_PRESENTER_H
#define SRC_PRESENTATION_TILE_SOURCE_WIDGET_PRESENTER_H

#include "src/model/TileModel.h"

#include <string>

namespace presentation {
class TileSourceWidgetPresenter {
public:
    TileSourceWidgetPresenter():
        model{model::TileModel::getInstance()}
    {}

    auto handleGetTileEngineList() const noexcept { return model.getTileEngineTypes(); }
    auto handleGetTileSourceList() const noexcept { return model.getTileSourceTypes(); }
    auto handleSetTileEngine(const std::string& engine) { return model.setTileEngine(engine); }

private:
    model::TileModel& model;
};
}

#endif
