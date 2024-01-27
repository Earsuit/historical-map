#ifndef SRC_TILE_TILEENGINEFACTORY
#define SRC_TILE_TILEENGINEFACTORY

#include "src/tile/TileEngine.h"

#include <memory>
#include <string>
#include <map>
#include <functional>

namespace tile {
class TileEngineFactory {
public:
    static std::shared_ptr<TileEngine> createInstance(const std::string& name);

protected:
    static std::map<std::string, std::function<std::shared_ptr<TileEngine>()>> creator;
};

template<typename T>
struct TileEngineRegister : TileEngineFactory { 
    TileEngineRegister(const std::string& s) { 
        creator.emplace(std::make_pair(s, [](){
            return std::make_shared<T>();
        }));
    }
};

}

#endif /* SRC_TILE_TILEENGINEFACTORY */
