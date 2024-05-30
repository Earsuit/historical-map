#ifndef SRC_PRESENTATION_MAP_WIDGET_PRESENTER_H
#define SRC_PRESENTATION_MAP_WIDGET_PRESENTER_H

#include "src/tile/Tile.h"
#include "src/presentation/Util.h"
#include "src/persistence/Data.h"
#include "src/presentation/Util.h"
#include "src/presentation/MapWidgetInterface.h"
#include "src/persistence/HistoricalStorage.h"
#include "src/model/TileModel.h"
#include "src/model/DatabaseModel.h"
#include "src/model/DynamicInfoModel.h"

#include "spdlog/spdlog.h"
#include "blockingconcurrentqueue.h"

#include <cstddef>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <thread>
#include <atomic>

namespace presentation {
class MapWidgetPresenter {
public:
    MapWidgetPresenter(MapWidgetInterface& view, const std::string& source);
    ~MapWidgetPresenter();

    void handleRenderTiles();
    void handleRenderCountry();
    void handleRenderCity();
    std::string handleGetOverlayText() const;
    bool handleRequestHasRightClickMenu() const noexcept;
    std::vector<std::string> handleRequestCountryList() const;
    bool handleExtendContour(const std::string& name, const model::Vec2& pos);
    bool handleAddCountry(const std::string& name, const model::Vec2& pos);
    bool handleAddCity(const std::string& name, const model::Vec2& pos);

private:
    std::shared_ptr<spdlog::logger> logger;
    MapWidgetInterface& view;
    model::DatabaseModel& databaseModel;
    model::TileModel& tileModel;
    model::DynamicInfoModel& dynamicInfoModel;
    moodycamel::BlockingConcurrentQueue<std::function<void()>> taskQueue;
    std::atomic_bool runWorkerThread;
    std::thread workerThread;
    std::string source;

    std::pair<persistence::Coordinate, model::Vec2> handleRenderCoordinate(persistence::Coordinate coordinate, const Color& color);
    void worker();
    void startWorkerThread();
    void stopWorkerThread();
};
}

#endif
