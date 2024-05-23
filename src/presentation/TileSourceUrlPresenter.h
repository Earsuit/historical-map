#ifndef SRC_PRESENTATION_TILE_SOURCE_URL_PRESENTER_H
#define SRC_PRESENTATION_TILE_SOURCE_URL_PRESENTER_H

#include "src/tile/TileSourceUrl.h"
#include "src/model/TileModel.h"

#include <string>
#include <memory>

namespace presentation {
class TileSourceUrlPresenter {
public:
    TileSourceUrlPresenter();

    std::string handleGetUrl() const noexcept { return url; }
    void handleSetUrl(const std::string& url);
    void handleSetTileSource();

private:
    model::TileModel& model;
    std::string url = "https://a.tile.openstreetmap.org/{Z}/{X}/{Y}.png";
};
}

#endif