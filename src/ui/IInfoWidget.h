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
    int getYear() const noexcept;

    virtual void drawRightClickMenu(float longitude, float latitude) = 0;
    virtual std::vector<HistoricalInfo> getInfo() = 0;
    virtual bool complete() = 0;

protected:
    int year;

private:
    virtual void historyInfo() = 0;
};
}

#endif /* SRC_UI_IINFOWIDGET */
