#ifndef SRC_UI_IMPORT_INFO_WIDGET_H
#define SRC_UI_IMPORT_INFO_WIDGET_H

#include "src/ui/IInfoWidget.h"
#include "src/presentation/InfoSelectorPresenter.h"
#include "src/presentation/HistoricalInfoPresenter.h"
#include "src/presentation/ImportYearPresenter.h"
#include "src/presentation/ImportPresenter.h"
#include "src/presentation/DatabaseSaverPresenter.h"
#include "src/logger/ModuleLogger.h"

#include <string>
#include <map>
#include <atomic>

namespace ui {
class ImportInfoWidget: public IInfoWidget {
public:
    ImportInfoWidget();
    ~ImportInfoWidget();

    virtual void paint() override;
    virtual bool complete() const noexcept override { return isComplete; }

    void setRefreshDatabaseCountries() noexcept { databaseCountryResourceUpdated = true; }
    void setRefreshDatabaseCities() noexcept { databaseCityResourceUpdated = true; }
    void setRefreshDatabaseNote() noexcept { databaseNoteResourceUpdated = true; }
    void setRefreshImportedCountries() noexcept { importedCountryResourceUpdated = true; }
    void setRefreshImportedCities() noexcept { importedCityResourceUpdated = true; }
    void setRefreshImportedNote() noexcept { importedNoteResourceUpdated = true; }
    void setRefreshSelectAll() noexcept { needUpdateSelectAll = true; }
    void setRefreshAll(int year) noexcept;

private:
    logger::ModuleLogger logger;
    presentation::HistoricalInfoPresenter databaseInfoPresenter;
    presentation::HistoricalInfoPresenter importInfoPresenter;
    presentation::InfoSelectorPresenter infoSelectorPresenter;
    presentation::ImportYearPresenter yearPresenter;
    presentation::ImportPresenter importPresenter;
    presentation::DatabaseSaverPresenter databaseSaverPresenter;
    std::atomic_int currentYear;
    bool importComplete = false;
    std::string errorMsg;
    bool openErrorPopup;
    bool isComplete = false;
    std::atomic_bool databaseCountryResourceUpdated = false;
    std::map<std::string, std::vector<persistence::Coordinate>> databaseCountries;
    std::atomic_bool databaseCityResourceUpdated = false;
    std::map<std::string, persistence::Coordinate> databaseCities;
    std::atomic_bool databaseNoteResourceUpdated = false;
    std::string databaseNote;
    std::atomic_bool importedCountryResourceUpdated = false;
    std::map<std::string, std::vector<persistence::Coordinate>> importedCountries;
    std::atomic_bool importedCityResourceUpdated = false;
    std::map<std::string, persistence::Coordinate> importedCities;
    std::atomic_bool importedNoteResourceUpdated = false;
    std::string importedNote;
    bool selectAll = false;
    std::atomic_bool needUpdateSelectAll = false;

    void displayYearControlSection();
    std::string fileExtensionFormat() const;
    void doImport();
    void selectCountry(const std::string& name);
    void displayCountry(const std::string& name, const std::vector<persistence::Coordinate>& contour);
    void selectCity(const std::string& name);
    void displayCity(const std::string& name, const persistence::Coordinate& coord);
    void selectNote(const std::string& note);
    void displayNote(const std::string& note);
    void displayCoordinate(const std::string& uniqueId, 
                           const persistence::Coordinate& coord);
    void displaySaveToDatabasePopup();

    void updateSelectAll();
    void updateCountryResources();
    void updateCityResources();
    void updateNoteResources();

    void updateCountryResources(const presentation::HistoricalInfoPresenter& presenter,
                                std::map<std::string, std::vector<persistence::Coordinate>>& cache);
    void updateCityResources(const presentation::HistoricalInfoPresenter& presenter, 
                             std::map<std::string, persistence::Coordinate>& cache);
    void updateNoteResources(const presentation::HistoricalInfoPresenter& presenter, std::string& cache);
};
}

#endif
