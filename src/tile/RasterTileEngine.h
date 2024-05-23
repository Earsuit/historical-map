#ifndef SRC_TILE_RASTERTILEENGINE
#define SRC_TILE_RASTERTILEENGINE

#include "src/tile/TileEngine.h"

namespace tile {
struct RasterTileEngine : public TileEngine {
public:
    Image toImage(const std::vector<std::byte>& rawBlob) override;
};
}

#endif /* SRC_TILE_RASTERTILEENGINE */
