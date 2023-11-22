#ifndef SRC_TILESOURCEURL
#define SRC_TILESOURCEURL

#include "TileSource.h"
#include "Tile.h"

#include <string>
#include <list>

class TileSoureUrl: public TileSource {
public:
    TileSoureUrl() = default;
    ~TileSoureUrl() override;

    bool request(int x, int y, int z) override;
    void waitAll() override;
    void isAllReady() override;
    void takeReady(std::vector<std::shared_ptr<Tile>>& tiles) override;

    void setUrl(const std::string& url);

private:
    std::string url;
    std::list<std::future<Tile>> requests;

    const char* makeUrl(int x, int y, int z);
};

#endif /* SRC_TILESOURCEURL */
