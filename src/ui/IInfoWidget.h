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

using MutableInfo = std::shared_ptr<persistence::Data>;
using Immutableinfo = std::shared_ptr<const persistence::Data>;
using HistoricalInfo = std::variant<MutableInfo, Immutableinfo>;

struct HistoricalInfoPack {
    HistoricalInfo info;
    std::string source;
};

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
    virtual std::vector<HistoricalInfoPack> getInfos() const = 0;
    virtual std::optional<persistence::Coordinate> getHovered() const noexcept = 0;
    virtual bool complete() const noexcept = 0;

private:
    int year;

    virtual int historyInfo(int year) = 0;
};
}

#endif /* SRC_UI_IINFOWIDGET */
