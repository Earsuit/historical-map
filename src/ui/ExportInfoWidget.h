#ifndef SRC_UI_EXPORT_INFO_WIDGET_H
#define SRC_UI_EXPORT_INFO_WIDGET_H

#include "src/ui/IInfoWidget.h"
#include "src/presentation/InfoSelectorPresenter.h"
#include "src/presentation/HistoricalInfoPresenter.h"
#include "src/presentation/DatabaseYearPresenter.h"
#include "src/presentation/ExportPresenter.h"
#include "src/logger/ModuleLogger.h"

#include <atomic>

namespace ui {
class ExportInfoWidget: public IInfoWidget {
public:
    ExportInfoWidget();
    ~ExportInfoWidget();

    virtual void paint() override;
    virtual bool complete() const noexcept override { return isComplete; }

    void setRefreshCountries() noexcept { countryResourceUpdated = true; }
    void setRefreshCities() noexcept { cityResourceUpdated = true; }
    void setRefreshNote() noexcept { noteResourceUpdated = true; }
    void setRefreshSelectAll() noexcept { needUpdateSelectAll = true; }
    void setRefreshAll(int year) noexcept;

private:
    logger::ModuleLogger logger;
    presentation::HistoricalInfoPresenter infoPresenter;
    presentation::InfoSelectorPresenter infoSelectorPresenter;
    presentation::DatabaseYearPresenter yearPresenter;
    presentation::ExportPresenter exportPresenter;
    std::atomic_int currentYear;
    bool selectAll = true;
    bool isComplete = false;
    std::string errorMsg;
    bool exportFailPopup = false;
    bool openFailPopup = false;
    bool exportComplete = false;
    int startYear;
    int endYear;
    bool processMultiYearSelection = false;
    bool processMultiYearSelectionComplete = false;
    std::atomic_bool countryResourceUpdated = false;
    std::map<std::string, std::vector<persistence::Coordinate>> countries;
    std::atomic_bool cityResourceUpdated = false;
    std::map<std::string, persistence::Coordinate> cities;
    std::atomic_bool noteResourceUpdated = false;
    std::string note;
    std::atomic_bool needUpdateSelectAll = false;

    void displayYearControlSection();
    void displayCountry(const std::string& name, const std::vector<persistence::Coordinate>& contour);
    void displayCity(const std::string& name, const persistence::Coordinate& coord);
    void displayNote();
    void displayCoordinate(const std::string& uniqueId, const persistence::Coordinate& coord);
    void displayExportPopup();
    void displaySelectAllForMultipleYearsPopup();

    void updateCountryResources();
    void updateCityResources();
    void updateNoteResources();
    void updateSelectAll();
};
}

#endif
