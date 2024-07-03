#ifndef SRC_UI_LOG_WIDGET_H
#define SRC_UI_LOG_WIDGET_H

#include "src/presentation/LogPresenter.h"
#include "src/presentation/LogWidgetInterface.h"

#include "imgui.h"
#include "concurrentqueue.h"

#include <array>
#include <string>
#include <optional>

namespace ui {
#define __(x) x     // gettext translation registration for constexpr

constexpr auto LOG_WIDGET_NAME = __("Log###Log");

class LogWidget : public presentation::LogWidgetInterface {
public:
    LogWidget();

    void paint();

    void displayLog(ImVec4 color, const std::string& msg) override;

private:
    presentation::LogPresenter presenter;
    std::array<std::string, presentation::NUM_LEVLES> levels;
    int logLevel;
    std::string filter;
    size_t logId = 0;
};
}

#endif
