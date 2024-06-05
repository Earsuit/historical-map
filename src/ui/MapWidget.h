#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "src/presentation/MapWidgetPresenter.h"
#include "src/presentation/MapWidgetInterface.h"
#include "src/logger/ModuleLogger.h"
#include "src/persistence/Data.h"
#include "src/ui/IInfoWidget.h"

#include "external/imgui/imgui.h"
#include "external/implot/implot.h"

#include <utility>
#include <optional>
#include <map>
#include <atomic>
#include <mutex>

namespace ui {
constexpr auto MAP_WIDGET_NAME_PREFIX = "Map plot";

class MapWidget: presentation::MapWidgetInterface {
public:
    MapWidget(const std::string& source);
    virtual ~MapWidget();

    void paint();

    virtual model::Range getAxisRangeX() const noexcept override;
    virtual model::Range getAxisRangeY() const noexcept override;
    virtual model::Vec2 getPlotSize() const noexcept override;
    virtual void renderTile(void* texture, const model::Vec2& bMin, const model::Vec2& bMax) override;
    virtual std::optional<model::Vec2> getMousePos() const override;

    std::string getName() const noexcept { return MAP_WIDGET_NAME_PREFIX + source; }

private:
    struct Country {
        ImVec4 color;
        std::vector<ImVec2> contour;
        ImVec2 labelCoordinate;
    };

    struct City {
        ImVec4 color;
        ImVec2 coordinate;
    };

    logger::ModuleLogger logger;
    presentation::MapWidgetPresenter presenter;
    std::string source;
    size_t dragPointId = 0;
    ImPlotRect plotRect;
    ImVec2 plotSize;
    std::optional<ImPlotPoint> mousePos;
    ImPlotPoint rightClickMenuPos;
    std::string plotName;
    std::string newCountryName;
    std::string newCityName;
    std::atomic_bool countryUpdated = true;
    std::atomic_bool cityUpdated = true;
    std::map<std::string, Country> countries;
    std::map<std::string, City> cities;
    std::mutex lock;
    std::vector<std::string> databaseCities;
    bool zoomIn = false;
    bool zoomOut = false;
    bool resetZoom = false;

    void renderMap();
    void renderOverlay();
    void renderCountries();
    void renderCities();
    void renderButtons();
    void updatCountries();
    void updateCities();

    void onCountryUpdate() noexcept { countryUpdated = true; }
    void onCityUpdate() noexcept { cityUpdated = true; }
    void onDatabaseCityListUpdate(std::vector<std::string>&& cities); 

    virtual void renderRightClickMenu();
    virtual void prepareRenderPoint();
    virtual bool renderPoint(ImVec2& coordinate, float size, const ImVec4& color);
};
}

#endif
