#ifndef SRC_UI_IINFOWIDGET
#define SRC_UI_IINFOWIDGET

namespace ui {
constexpr auto INFO_WIDGET_NAME = "Info widget";

class IInfoWidget {
public:
    virtual ~IInfoWidget() = default;

    virtual void paint() = 0;
    virtual bool complete() const noexcept = 0;
};
}

#endif /* SRC_UI_IINFOWIDGET */
