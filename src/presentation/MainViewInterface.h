#ifndef SRC_PRESENTATION_MAIN_VIEW_INTERFACE_H
#define SRC_PRESENTATION_MAIN_VIEW_INTERFACE_H

#include <string>

namespace presentation {
class MainViewInterface {
public:
    virtual void addInteractiveMapWidget(const std::string& source) = 0;
    virtual void addNoninteractiveMapWidget(const std::string& source) = 0;
    virtual void clearMapWidgets() = 0;
};
}

#endif
