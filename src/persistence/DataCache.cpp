#include "src/persistence/DataCache.h"
#include "src/logger/Util.h"

namespace persistence {
DataCache::DataCache(int size):
    logger{spdlog::get(logger::LOGGER_NAME)},
    size{size}
{
}

Data& DataCache::operator[](int year)
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

bool DataCache::contains(int year)
{
    return cache.contains(year);
}

}