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
#include "src/util/Signal.h"

#include "spdlog/spdlog.h"
#include "imgui.h"

#include <cstddef>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <optional>
#include <atomic>

namespace presentation {
class MapWidgetPresenter {
public:
    MapWidgetPresenter(MapWidgetInterface& view, const std::string& source);
    ~MapWidgetPresenter();

    std::vector<std::string> handleRequestCountryList() const;
    std::vector<ImVec2> handleRequestContour(const std::string& name) const;
    ImVec4 handleRequestColor(const std::string& name) const;
    std::vector<std::string> handleRequestCityList() const;
    std::optional<ImVec2> handleRequestCityCoord(const std::string& name) const;
    float handleRequestCoordSize(const ImVec2& coord) const;
    void handleUpdateContour(const std::string& name, int idx, const ImVec2& coord);
    void handleUpdateCity(const std::string& name, const ImVec2& coord);

    void handleRenderTiles();
    std::string handleGetOverlayText() const;
    bool handleRequestHasRightClickMenu() const noexcept;
    bool handleExtendContour(const std::string& name, const model::Vec2& pos);
    bool handleAddCountry(const std::string& name, const model::Vec2& pos);
    bool handleAddCity(const std::string& name, const model::Vec2& pos);

    util::signal::Signal<void()> countryUpdated;
    util::signal::Signal<void()> cityUpdated;

private:
    std::shared_ptr<spdlog::logger> logger;
    MapWidgetInterface& view;
    model::DatabaseModel& databaseModel;
    model::TileModel& tileModel;
    model::DynamicInfoModel& dynamicInfoModel;
    std::string source;
    std::atomic_int year;

    void onCountryUpdate(const std::string& source, int year);
    void onCityUpdate(const std::string& source, int year);
    void onYearChange(int year);
    bool varifySignal(const std::string& source, int year) const noexcept;
};
}

#endif
