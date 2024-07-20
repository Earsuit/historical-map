#ifndef SRC_TILE_TILE_SOURCE_URL_H
#define SRC_TILE_TILE_SOURCE_URL_H

#include "TileSource.h"
#include "src/logger/ModuleLogger.h"

#include <string>
#include <list>
#include <future>
#include <cstddef>
#include <atomic>

namespace tile {

class TileSourceUrl: public TileSource {
public:
    TileSourceUrl(const std::string& url);
    ~TileSourceUrl() override = default;

    std::vector<std::byte> request(const Coordinate& coord) override;
    void stopAllRequests() override;

    bool setUrl(const std::string& url);

private:
    std::string url;
    std::atomic_bool stop = false;

    logger::ModuleLogger logger;
    const std::string makeUrl(const Coordinate& coord);
};

}

#endif
