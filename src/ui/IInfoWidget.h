#ifndef SRC_UI_IINFOWIDGET
#define SRC_UI_IINFOWIDGET

#include "src/persistence/Data.h"

#include <tuple>
#include <memory>
#include <optional>
#include <vector>

namespace ui {
using HistoricalInfo = std::tuple<std::string, std::shared_ptr<persistence::Data>, std::optional<persistence::Coordinate>>;

class IInfoWidget {
public:
    virtual ~IInfoWidget() = default;

    virtual void paint() = 0;
    virtual void drawRightClickMenu(float longitude, float latitude) = 0;
    virtual std::vector<HistoricalInfo> getInfo() = 0;
};
}

#endif /* SRC_UI_IINFOWIDGET */
