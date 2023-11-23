#ifndef SRC_TILE_TILE_SOURCE_URL_H
#define SRC_TILE_TILE_SOURCE_URL_H

#include "TileSource.h"
#include "Tile.h"

#include <string>
#include <list>
#include <memory>

class TileSoureUrl: public TileSource {
public:
    TileSoureUrl() = default;
    ~TileSoureUrl() override;

    void request(int x, int y, int z) override;
    void waitAll() override;
    bool isAllReady() override;
    void takeReady(std::vector<std::shared_ptr<Tile>>& tiles) override;

    void setUrl(const std::string& url);

private:
    std::string url;
    std::list<std::future<std::shared_ptr<Tile>>> requests;

    const char* makeUrl(int x, int y, int z);
};

#endif
