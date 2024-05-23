#include "src/presentation/MainViewPresenter.h"
#include "src/presentation/Util.h"

namespace presentation {
std::string MainViewPresenter::handleGetDefaultMapWidgetSouceName() const noexcept
{
    return DEFAULT_HISTORICAL_INFO_SOURCE;
}

void MainViewPresenter::handleClickImport()
{
    clearCache();
    view.clearMapWidgets();
    view.addNoninteractiveMapWidget(DEFAULT_HISTORICAL_INFO_SOURCE);
    view.addNoninteractiveMapWidget(IMPORT_SOURCE);
}

void MainViewPresenter::handleClickExport()
{
    clearCache();
    view.clearMapWidgets();
    view.addNoninteractiveMapWidget(DEFAULT_HISTORICAL_INFO_SOURCE);
}

void MainViewPresenter::handleImportExportComplete()
{
    view.clearMapWidgets();
    view.addInteractiveMapWidget(DEFAULT_HISTORICAL_INFO_SOURCE);
}

void MainViewPresenter::clearCache()
{
    model.removeHistoricalInfoFromSource(DEFAULT_HISTORICAL_INFO_SOURCE);
}
}