#include "src/ui/MapWidgetNoninteractive.h"
#include "src/model/Util.h"

#include "external/implot/implot.h"

namespace ui {
void MapWidgetNoninteractive::prepareRenderPoint()
{
    dragPointId = 0;
}

bool MapWidgetNoninteractive::renderPoint(ImVec2& coordinate, float size, const ImVec4& color)
{
    double x = coordinate.x;
    double y = coordinate.y;
    ImPlot::DragPoint(dragPointId++, 
                      &x, 
                      &y, 
                      color, 
                      size, 
                      ImPlotDragToolFlags_NoInputs);

    return false;
}
}