#ifndef SRC_PRESENTATION_HISTORICAL_INFO_WIDGET_INTERFACE_H
#define SRC_PRESENTATION_HISTORICAL_INFO_WIDGET_INTERFACE_H

#include "src/persistence/Data.h"

#include <string>

namespace presentation {
class HistoricalInfoWidgetInterface {
public:
    virtual void displayCountry(const std::string& name) = 0;
    virtual void displayCity(const std::string& name) = 0;
    virtual persistence::Coordinate displayCoordinate(const persistence::Coordinate& coord) = 0;
};
}

#endif
