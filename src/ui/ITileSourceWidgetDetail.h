#ifndef SRC_UI_ITILE_SOURCE_WIDGET_H
#define SRC_UI_ITILE_SOURCE_WIDGET_H

namespace ui {
class ITileSourceWidgetDetail {
public:
    virtual ~ITileSourceWidgetDetail() = default;
    virtual void paint() = 0;
};
}

#endif /* SRC_UI_ITILE_SOURCE_WIDGET_H */
