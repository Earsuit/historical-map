#ifndef SRC_TILE_TILE_SOURCE_URL_H
#define SRC_TILE_TILE_SOURCE_URL_H

#include "TileSource.h"
#include "Tile.h"

#include "spdlog/spdlog.h"

#include <string>
#include <list>
#include <memory>
#include <future>

namespace tile {

class TileSourceUrl: public TileSource {
public:
    TileSourceUrl(const std::string& url);
    ~TileSourceUrl() override = default;

    std::future<std::shared_ptr<Tile>> request(const Coordinate& coord) override;

    bool setUrl(const std::string& url);

private:
    struct Url {
        std::string url = {};
        std::string::size_type zoomPos = 0;
        std::string::size_type xPos = 0;
        std::string::size_type yPos = 0;
    };

    Url url;

    std::shared_ptr<spdlog::logger> logger;
    const std::string makeUrl(const Coordinate& coord);
};

}

#endif
