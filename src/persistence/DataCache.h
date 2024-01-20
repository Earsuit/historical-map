#ifndef SRC_PERSISTENCE_DATACACHE
#define SRC_PERSISTENCE_DATACACHE

#include "src/persistence/Data.h"

#include "spdlog/spdlog.h"

#include <map>
#include <queue>

namespace persistence {
class DataCache {
public:
    DataCache(int size);

    Data& operator[](int year);

    bool contains(int year);

private:
    std::shared_ptr<spdlog::logger> logger;
    int size;
    std::map<int, Data> cache;
    std::queue<int> lifetime;
};
}

#endif /* SRC_PERSISTENCE_DATACACHE */
