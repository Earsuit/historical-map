#ifndef SRC_TILESOURCEURL
#define SRC_TILESOURCEURL

#include "TileSource.h"

#include <string>

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
};

#endif /* SRC_TILESOURCEURL */
