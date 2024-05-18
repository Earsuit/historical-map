#ifndef SRC_UI_EXPORT_INFO_WIDGET_H
#define SRC_UI_EXPORT_INFO_WIDGET_H

#include "src/ui/IInfoWidget.h"
#include "src/presentation/InfoSelectorPresenter.h"
#include "src/presentation/HistoricalInfoPresenter.h"
#include "src/presentation/DatabaseYearPresenter.h"

#include "spdlog/spdlog.h"

namespace ui {
class ExportInfoWidget: public IInfoWidget {
public:
    ExportInfoWidget();

    virtual void paint() override;
    virtual bool complete() const noexcept override { return isComplete; }

private:
    std::shared_ptr<spdlog::logger> logger;
    presentation::HistoricalInfoPresenter infoPresenter;
    presentation::InfoSelectorPresenter infoSelectorPresenter;
    presentation::DatabaseYearPresenter yearPresenter;
    int currentYear;
    bool selectAll;
    bool isComplete = false;

    void displayYearControlSection();
    void displayCountry(const std::string& name);
    void displayCity(const std::string& name);
    void displayNote();
    void displayCoordinate(const std::string& uniqueId, const persistence::Coordinate& coord);
};
}

#endif
