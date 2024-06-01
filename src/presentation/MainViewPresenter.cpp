#include "src/presentation/MainViewPresenter.h"
#include "src/presentation/Util.h"

namespace presentation {
std::string MainViewPresenter::handleGetDefaultMapWidgetSouceName() const noexcept
{
    return model::PERMENANT_SOURCE;
}

void MainViewPresenter::handleClickImport()
{
    clearCache();
    view.clearMapWidgets();
    view.addNoninteractiveMapWidget(model::PERMENANT_SOURCE);
    view.addNoninteractiveMapWidget(IMPORT_SOURCE);
}

void MainViewPresenter::handleClickExport()
{
    clearCache();
    view.clearMapWidgets();
    view.addNoninteractiveMapWidget(model::PERMENANT_SOURCE);
}

void MainViewPresenter::handleImportExportComplete()
{
    view.clearMapWidgets();
    view.addInteractiveMapWidget(model::PERMENANT_SOURCE);
}

void MainViewPresenter::clearCache()
{
    model.removeHistoricalInfoFromSource(model::PERMENANT_SOURCE, databaseModel.getYear());
}
}