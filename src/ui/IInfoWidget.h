#ifndef SRC_UI_IINFOWIDGET
#define SRC_UI_IINFOWIDGET

#include "src/persistence/Data.h"

#include <utility>
#include <memory>
#include <optional>

namespace ui {
class IInfoWidget {
public:
    virtual ~IInfoWidget() = default;

    virtual void paint() = 0;
    virtual void drawRightClickMenu(float longitude, float latitude) = 0;
    virtual std::pair<std::shared_ptr<persistence::Data>, std::optional<persistence::Coordinate>> getInfo() = 0;
};
}

#endif /* SRC_UI_IINFOWIDGET */
