#ifndef SRC_PRESENTATION_MAIN_VIEW_PRESENTER_H
#define SRC_PRESENTATION_MAIN_VIEW_PRESENTER_H

#include "src/presentation/MainViewInterface.h"
#include "src/model/CacheModel.h"
#include "src/model/DatabaseModel.h"

#include <string>

namespace presentation {
class MainViewPresenter {
public:
    MainViewPresenter(MainViewInterface& view):
        view{view},
        databaseModel{model::DatabaseModel::getInstance()},
        model{model::CacheModel::getInstance()}
    {}

    std::string handleGetDefaultMapWidgetSouceName() const noexcept;
    void handleClickImport();
    void handleClickExport();
    void handleImportExportComplete();

private:
    MainViewInterface& view;
    model::DatabaseModel& databaseModel;
    model::CacheModel& model;

    void clearCache();
};
}

#endif
