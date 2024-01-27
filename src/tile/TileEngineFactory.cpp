#include "src/tile/TileEngineFactory.h"
#include "src/tile/RasterTileEngine.h"

namespace tile {
std::shared_ptr<TileEngine> TileEngineFactory::createInstance(const std::string& name)
{
    if (creator.contains(name)) {
        return creator[name]();
    } else {
        return nullptr;
    }
}

std::map<std::string, std::function<std::shared_ptr<TileEngine>()>> TileEngineFactory::creator{};
static TileEngineRegister<RasterTileEngine> RasterTileEngineRegister{RASTER_TILE_ENGINE_NAME};
}