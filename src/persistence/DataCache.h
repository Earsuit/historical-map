#ifndef SRC_PERSISTENCE_DATACACHE
#define SRC_PERSISTENCE_DATACACHE

#include "src/persistence/Data.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <map>
#include <queue>

namespace persistence {
template<typename T>
class DataCache {
public:
    DataCache(int size):
        logger{spdlog::get(logger::LOGGER_NAME)},
        size{size}
    {
    }

    T& operator[](int year)
    {
        if (!cache.contains(year)) {
            // there is no such year in cache before,
            // so this time must be an insert
            if (cache.size() == size) {
                logger->debug("Data cache full, remove oldest cache {}", lifetime.front());
                cache.erase(lifetime.front());
                lifetime.pop();
            }
            
            logger->debug("Insert {} data into cache", year);
            lifetime.push(year);   
        }

        return cache[year];
    }

    bool contains(int year)
    {
        return cache.contains(year);
    }

private:
    std::shared_ptr<spdlog::logger> logger;
    int size;
    std::map<int, T> cache;
    std::queue<int> lifetime;
};
}

#endif /* SRC_PERSISTENCE_DATACACHE */
