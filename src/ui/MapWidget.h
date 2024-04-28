#ifndef SRC_UI_MAP_PLOT_H
#define SRC_UI_MAP_PLOT_H

#include "src/presentation/MapWidgetPresenter.h"
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

constexpr auto MAP_WIDGET_NAME = "Map plot";

class MapWidget {
public:
    MapWidget(): 
        logger{spdlog::get(logger::LOGGER_NAME)}
    {}

    void paint(IInfoWidget& infoWidget);

private:
    std::shared_ptr<spdlog::logger> logger;
    presentation::MapWidgetPresenter presenter;

    std::optional<ImPlotPoint> renderMap(IInfoWidget& infoWidget, 
                                         ImVec2 size, 
                                         const std::string& name, 
                                         HistoricalInfo historicalInfo, 
                                         std::optional<persistence::Coordinate> hovered);
    void renderOverlay(const std::string& name, int offset, const std::optional<ImPlotPoint>& mousePos);
    void renderHistoricalInfo(HistoricalInfo historicalInfo, std::optional<persistence::Coordinate> hovered);
    
    template<typename T>
    std::pair<double, double> renderCoordinate(T& coordinate, const ImVec4& color, float size, int id)
    {
        double x = presenter.longitude2X(coordinate.longitude);
        double y = presenter.latitude2Y(coordinate.latitude);
        
        if constexpr (std::is_const_v<T>) {
            ImPlot::DragPoint(id, &x, &y, color, size, ImPlotDragToolFlags_NoInputs);
        } else {
            if (ImPlot::DragPoint(id, &x, &y, color, size)) {
                coordinate.latitude = presenter.y2Latitude(static_cast<float>(y));
                coordinate.longitude = presenter.x2Longitude(static_cast<float>(x));
            }
        }

        logger->trace("Lon {}, lat {} at [{}, {}]",
                    coordinate.longitude,
                    coordinate.latitude,
                    x,
                    y);

        return {x, y};
    }
};
}

#endif
