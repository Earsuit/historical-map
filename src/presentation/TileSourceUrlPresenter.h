#ifndef SRC_PRESENTATION_TILE_SOURCE_URL_PRESENTER_H
#define SRC_PRESENTATION_TILE_SOURCE_URL_PRESENTER_H

#include "src/tile/TileSourceUrl.h"
#include "src/model/TileModel.h"

#include <string>
#include <memory>

namespace presentation {
constexpr auto DEFAULT_URL = "https://a.tile.openstreetmap.org/{Z}/{X}/{Y}.png";

class TileSourceUrlPresenter {
public:
    TileSourceUrlPresenter();

    std::string handleGetDefaultUrl() const noexcept { return DEFAULT_URL; }
    bool handleSetUrl(const std::string& url);

private:
    model::TileModel& model;
    std::string url = DEFAULT_URL;
    std::shared_ptr<tile::TileSourceUrl> source;
};
}

#endif
