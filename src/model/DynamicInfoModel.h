#ifndef SRC_MODEL_DYNAMIC_INFO_MODEL_H
#define SRC_MODEL_DYNAMIC_INFO_MODEL_H

#include "src/persistence/Data.h"
#include "src/persistence/HistoricalStorage.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace model {
class DynamicInfoModel {
public:
    static DynamicInfoModel& getInstance();

    void setHoveredCoord(persistence::Coordinate coord);
    void clearHoveredCoord() noexcept;
    std::optional<persistence::Coordinate> getHoveredCoord() const;

    bool addSource(const std::string& source);
    std::vector<std::string> getSourceList() const noexcept;
    void remove(const std::string& source);
    void clearHistoricalInfoFromSource(const std::string& source);
    std::shared_ptr<persistence::HistoricalStorage> getHistoricalInfo(const std::string& source);

    template<typename T>
    bool upsert(const std::string& source, T&& info)
    {
        std::lock_guard lk(cacheLock);
        if (cache.contains(source)) {
            cache[source] = std::make_shared<persistence::HistoricalStorage>(std::forward<T>(info));
            return true;
        }

        return false;
    }

    DynamicInfoModel(DynamicInfoModel&&) = delete;
    DynamicInfoModel(const DynamicInfoModel&) = delete;
    DynamicInfoModel& operator=(const DynamicInfoModel&) = delete;

private:
    DynamicInfoModel():
        logger{spdlog::get(logger::LOGGER_NAME)}
    {
    }

    std::shared_ptr<spdlog::logger> logger;
    std::optional<persistence::Coordinate> hovered;
    mutable std::mutex hoveredLock;
    mutable std::mutex cacheLock;
    mutable std::mutex removeLock;
    std::map<std::string, std::shared_ptr<persistence::HistoricalStorage>> cache;
    persistence::Data removed;
};
}

#endif
