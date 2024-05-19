#ifndef SRC_UI_EXPORT_INFO_WIDGET_H
#define SRC_UI_EXPORT_INFO_WIDGET_H

#include "src/ui/IInfoWidget.h"
#include "src/presentation/InfoSelectorPresenter.h"
#include "src/presentation/HistoricalInfoPresenter.h"
#include "src/presentation/DatabaseYearPresenter.h"
#include "src/presentation/ExportPresenter.h"

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
    presentation::ExportPresenter exportPresenter;
    int currentYear;
    bool selectAll;
    bool isComplete = false;
    std::string errorMsg;
    bool exportFailPopup = false;
    bool openFailPopup = false;
    bool exportComplete = false;
    int startYear;
    int endYear;
    bool processMultiYearSelection = false;
    bool processMultiYearSelectionComplete = false;

    void displayYearControlSection();
    void displayCountry(const std::string& name);
    void displayCity(const std::string& name);
    void displayNote();
    void displayCoordinate(const std::string& uniqueId, const persistence::Coordinate& coord);
    void displayExportPopup();
    void displaySelectAllForMultipleYearsPopup();
};
}

#endif
