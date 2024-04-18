#ifndef SRC_UI_EXPORTWIDGET
#define SRC_UI_EXPORTWIDGET

#include "src/ui/ExportImportWidget.h"
#include "src/persistence/DatabaseManager.h"
#include "src/persistence/exporterImporter/ExportManager.h"
#include "src/persistence/Selector.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <future>

namespace ui {
class ExportWidget: public ExportImportWidget {
public:
    ExportWidget(int year):
        ExportImportWidget{year},
        logger{spdlog::get(logger::LOGGER_NAME)}, 
        database{persistence::DatabaseManager::getInstance()}
    {
    }

    void drawRightClickMenu(float longitude, float latitude) override {};
    bool complete() const noexcept override;

private:
    std::shared_ptr<spdlog::logger> logger;
    persistence::DatabaseManager& database;
    persistence::ExportManager exporter;
    std::future<tl::expected<void, persistence::Error>> exportTask;
    bool exportComplete = false;
    std::shared_ptr<const persistence::Data> cache;
    bool isComplete = false;
    std::string exportFormat;
    std::string errorMsg;
    bool exportFailPopup = false;
    int currentYear;

    virtual bool cacheReady() const noexcept override;
    virtual int overwriteYear(int year) override;
    virtual std::optional<HistoricalInfoPack> getSelectableInfo() const override;
    virtual std::optional<HistoricalInfoPack> getUnselectableInfo() const override;
    virtual void doExportImport(const persistence::Selector& selector) override;
    virtual void buttons() override;
    virtual void updateInfo() override;

    void checkExportProgress();
};
}

#endif /* SRC_UI_EXPORTWIDGET */