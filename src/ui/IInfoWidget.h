#ifndef SRC_UI_IINFOWIDGET
#define SRC_UI_IINFOWIDGET

#include "src/persistence/Data.h"

#include <tuple>
#include <memory>
#include <optional>
#include <vector>
#include <string>

namespace ui {
constexpr auto INFO_WIDGET_NAME = "Info widget";

using HistoricalInfo = std::tuple<std::string, std::shared_ptr<persistence::Data>, std::optional<persistence::Coordinate>>;

class IInfoWidget {
public:
    IInfoWidget(int year):
        year{year}
    {
    }

    virtual ~IInfoWidget() = default;

    void paint();
    void drawRightClickMenu(float longitude, float latitude);
    std::vector<HistoricalInfo> getInfo();

protected:
    int year;

private:
    virtual void historyInfo() = 0;
    virtual void rightClickMenu(float longitude, float latitude) = 0;
    virtual std::vector<HistoricalInfo> getInfoImpl() = 0;
};
}

#endif /* SRC_UI_IINFOWIDGET */
