#ifndef SRC_UI_IMPORT_INFO_WIDGET_H
#define SRC_UI_IMPORT_INFO_WIDGET_H

#include "src/ui/IInfoWidget.h"
#include "src/presentation/InfoSelectorPresenter.h"
#include "src/presentation/HistoricalInfoPresenter.h"
#include "src/presentation/ImportYearPresenter.h"
#include "src/presentation/ImportPresenter.h"
#include "src/presentation/DatabaseSaverPresenter.h"

#include "spdlog/spdlog.h"

#include <string>
#include <memory>

namespace ui {
class ImportInfoWidget: public IInfoWidget {
public:
    ImportInfoWidget();

    virtual void paint() override;
    virtual bool complete() const noexcept override { return isComplete; }

private:
    std::shared_ptr<spdlog::logger> logger;
    presentation::HistoricalInfoPresenter databaseInfoPresenter;
    presentation::HistoricalInfoPresenter importInfoPresenter;
    presentation::InfoSelectorPresenter infoSelectorPresenter;
    presentation::ImportYearPresenter yearPresenter;
    presentation::ImportPresenter importPresenter;
    presentation::DatabaseSaverPresenter databaseSaverPresenter;
    int currentYear;
    bool importComplete;
    std::string errorMsg;
    bool openErrorPopup;
    bool isComplete = false;
    bool selectAll;

    void displayYearControlSection();
    std::string fileExtensionFormat() const;
    void doImport();
    void selectCountry(const std::string& name);
    void displayCountry(presentation::HistoricalInfoPresenter& infoPresenter, const std::string& name);
    void selectCity(const std::string& name);
    void displayCity(presentation::HistoricalInfoPresenter& infoPresenter, const std::string& name);
    void selectNote(const presentation::HistoricalInfoPresenter& infoPresenter);
    void displayNote(const presentation::HistoricalInfoPresenter& infoPresenter);
    void displayCoordinate(presentation::HistoricalInfoPresenter& infoPresenter, 
                           const std::string& uniqueId, 
                           const persistence::Coordinate& coord);
    void displaySaveToDatabasePopup();
};
}

#endif
