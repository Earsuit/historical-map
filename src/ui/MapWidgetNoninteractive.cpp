#include "src/ui/MapWidgetNoninteractive.h"
#include "src/model/Util.h"

#include "external/implot/implot.h"

namespace ui {
constexpr int POINT_NUMER = 1;

bool MapWidgetNoninteractive::renderPoint(ImVec2& coordinate, float size, const ImVec4& color)
{
    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, size, color);
    ImPlot::PlotScatter("##", &coordinate.x, &coordinate.y, POINT_NUMER);

    return false;
}
}