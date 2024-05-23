#include "src/ui/MapWidgetNoninteractive.h"
#include "src/model/Util.h"

#include "external/implot/implot.h"

namespace ui {
void MapWidgetNoninteractive::prepareRenderPoint()
{
    dragPointId = 0;
}

model::Vec2 MapWidgetNoninteractive::renderPoint(const model::Vec2& coordinate, float size, const presentation::Color& color)
{
    double x = coordinate.x;
    double y = coordinate.y;
    ImPlot::DragPoint(dragPointId++, &x, &y, ImVec4{color.red, color.green, color.blue, color.alpha}, size, ImPlotDragToolFlags_NoInputs);

    return {static_cast<float>(x), static_cast<float>(y)};
}
}