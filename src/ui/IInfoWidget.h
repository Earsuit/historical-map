#ifndef SRC_UI_IINFOWIDGET
#define SRC_UI_IINFOWIDGET

#include "src/persistence/Data.h"
#include "src/presentation/InfoWidgetPresenter.h"

#include <tuple>
#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <variant>

namespace ui {
constexpr auto INFO_WIDGET_NAME = "Info widget";

class IInfoWidget {
public:
    virtual ~IInfoWidget() = default;

    void paint();
    virtual bool complete() const noexcept = 0;

private:
    presentation::InfoWidgetPresenter presenter;

    virtual void historyInfo(int year) = 0;
};
}

#endif /* SRC_UI_IINFOWIDGET */
