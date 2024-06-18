#ifndef SRC_PRESENTATION_MAIN_VIEW_PRESENTER_H
#define SRC_PRESENTATION_MAIN_VIEW_PRESENTER_H

#include "src/presentation/MainViewInterface.h"
#include "src/model/CacheModel.h"
#include "src/model/DatabaseModel.h"
#include "src/logger/ModuleLogger.h"

#include <string>
#include <filesystem>

namespace presentation {
class MainViewPresenter {
public:
    MainViewPresenter(MainViewInterface& view);

    std::string handleGetDefaultMapWidgetSouceName() const noexcept;
    void handleClickImport();
    void handleClickExport();
    void handleImportExportComplete();
    void handleSetLanguage(const std::string& language);
    std::vector<std::string> handleGetLanguages() const;

private:
    MainViewInterface& view;
    model::DatabaseModel& databaseModel;
    model::CacheModel& model;
    logger::ModuleLogger logger;
    std::filesystem::path executableDirectory;

    void clearCache();
};
}

#endif
