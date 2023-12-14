#ifndef SRC_TILE_TILE_SOURCE_URL_H
#define SRC_TILE_TILE_SOURCE_URL_H

#include "TileSource.h"
#include "Tile.h"

#include <string>
#include <list>
#include <memory>
#include <future>

namespace tile {

class TileSourceUrl: public TileSource {
public:
    TileSourceUrl() = default;
    TileSourceUrl(const std::string& url);
    ~TileSourceUrl() override = default;

    void request(int x, int y, int z) override;
    void waitAll() override;
    bool isAllReady() override;
    void takeReady(std::vector<std::shared_ptr<Tile>>& tiles) override;

    bool setUrl(const std::string& url);

private:
    struct Url {
        std::string url = {};
        std::string::size_type zoomPos = 0;
        std::string::size_type xPos = 0;
        std::string::size_type yPos = 0;
    };

    Url url;
    std::list<std::future<std::shared_ptr<Tile>>> requests;

    const char* makeUrl(int x, int y, int z);
};

}

#endif
