#include "src/tile/TileEngineFactory.h"
#include "src/tile/RasterTileEngine.h"

#include <ranges>

namespace tile {
constexpr auto RASTER_TILE_ENGINE_NAME = "Raster Tile";
 
std::shared_ptr<TileEngine> TileEngineFactory::createInstance(const std::string& name)
{
    if (creator.contains(name)) {
        return creator[name]();
    } else {
        return nullptr;
    }
}

std::vector<std::string> TileEngineFactory::getTileEngines()
{
    std::vector<std::string> engines;
    for (const auto& engine : std::views::keys(creator)) {
        engines.emplace_back(engine);
    }

    return engines;
}

std::map<std::string, std::function<std::shared_ptr<TileEngine>()>> TileEngineFactory::creator{};
static TileEngineRegister<RasterTileEngine> RasterTileEngineRegister{RASTER_TILE_ENGINE_NAME};
}