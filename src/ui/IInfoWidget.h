#ifndef SRC_UI_IINFOWIDGET
#define SRC_UI_IINFOWIDGET

#include "src/persistence/Data.h"

#include <tuple>
#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <variant>

namespace ui {
constexpr auto INFO_WIDGET_NAME = "Info widget";

using MutableData = std::shared_ptr<persistence::Data>;
using ImmutableData = std::shared_ptr<const persistence::Data>;
using HistoricalData = std::variant<MutableData, ImmutableData>;
using HistoricalInfo = std::tuple<std::string, HistoricalData, std::optional<persistence::Coordinate>>;

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

private:
    int year;

    virtual int historyInfo(int year) = 0;
};
}

#endif /* SRC_UI_IINFOWIDGET */
