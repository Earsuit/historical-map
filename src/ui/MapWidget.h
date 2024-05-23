#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "src/presentation/MapWidgetPresenter.h"
#include "src/presentation/MapWidgetInterface.h"
#include "src/logger/Util.h"
#include "src/persistence/Data.h"
#include "src/ui/IInfoWidget.h"

#include "external/imgui/imgui.h"
#include "spdlog/spdlog.h"
#include "external/implot/implot.h"

#include <utility>
#include <memory>
#include <optional>

namespace ui {
constexpr auto MAP_WIDGET_NAME_PREFIX = "Map plot";

class MapWidget: presentation::MapWidgetInterface {
public:
    MapWidget(const std::string& source): 
        logger{spdlog::get(logger::LOGGER_NAME)},
        presenter{*this, source},
        source{source},
        plotName{"##" + source}
    {
    }
    virtual ~MapWidget() = default;

    void paint();

    virtual void renderAnnotation(const model::Vec2& coordinate, 
                                  const std::string& name, 
                                  const presentation::Color& color, 
                                  const model::Vec2& offset) override;
    virtual model::Vec2 renderPoint(const model::Vec2& coordinate, float size, const presentation::Color& color) override;
    virtual void renderContour(const std::string& name, const std::vector<model::Vec2>& contour, const presentation::Color& color) override;
    virtual model::Range getAxisRangeX() const noexcept override;
    virtual model::Range getAxisRangeY() const noexcept override;
    virtual model::Vec2 getPlotSize() const noexcept override;
    virtual void renderTile(void* texture, const model::Vec2& bMin, const model::Vec2& bMax) override;
    virtual std::optional<model::Vec2> getMousePos() const override;

    std::string getName() const noexcept { return MAP_WIDGET_NAME_PREFIX + source; }

private:
    std::shared_ptr<spdlog::logger> logger;
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

    void renderMap();
    void renderOverlay();
    virtual void renderRightClickMenu();
    virtual void prepareRenderPoint();
};
}

#endif
