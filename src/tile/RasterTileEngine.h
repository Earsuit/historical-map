#ifndef SRC_TILE_RASTERTILEENGINE
#define SRC_TILE_RASTERTILEENGINE

#include "src/tile/TileEngine.h"

namespace tile {
constexpr auto RASTER_TILE_ENGINE_NAME = "Raster Tile";

struct RasterTileEngine : public TileEngine {
public:
    Image toImage(const std::vector<std::byte>& rawBlob) override;
};
}

#endif /* SRC_TILE_RASTERTILEENGINE */
