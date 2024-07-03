#ifndef SRC_PRESENTATION_LOG_WIDGET_INTERFACE_H
#define SRC_PRESENTATION_LOG_WIDGET_INTERFACE_H

#include "external/imgui/imgui.h"

#include <string>

namespace presentation {
class LogWidgetInterface {
public:
    virtual void displayLog(ImVec4 color, const std::string& msg) = 0;
};
}

#endif /* SRC_PRESENTATION_LOG_WIDGET_INTERFACE_H */
