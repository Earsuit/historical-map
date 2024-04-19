#ifndef SRC_UI_IMPORT_WIDGET_H
#define SRC_UI_IMPORT_WIDGET_H

#include "src/persistence/DatabaseManager.h"
#include "src/persistence/exporterImporter/ImportManager.h"
#include "src/persistence/exporterImporter/Util.h"
#include "src/persistence/Selector.h"
#include "src/ui/ExportImportWidget.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"

#include <memory>
#include <optional>
#include <atomic>

namespace ui {
class ImportWidget: public ExportImportWidget {
public:
    ImportWidget(int year);

    virtual void drawRightClickMenu(float longitude, float latitude) override {};
    virtual bool complete() const noexcept override;

private:
    std::shared_ptr<spdlog::logger> logger;
    persistence::DatabaseManager& database;
    std::shared_ptr<const persistence::Data> cache;
    persistence::ImportManager importer;
    persistence::Selector selection;
    std::future<tl::expected<void, persistence::Error>> importTask;
    std::future<void> writeToDatabaseTask;
    int currentYear;
    bool importFail = false;
    bool importComplete = false;
    bool isComplete = false;
    bool writeToDatabaseDone = false;
    bool moveYearToFirst = false;
    std::atomic<float> progress;
    std::string importProgressMessage;

    virtual bool cacheReady() const noexcept override;
    virtual int overwriteYear(int year) override;
    virtual std::optional<HistoricalInfoPack> getSelectableInfo() const override;
    virtual std::optional<HistoricalInfoPack> getUnselectableInfo() const override;
    virtual void doExportImport(const persistence::Selector& selector) override;
    virtual void buttons() override;
    virtual void updateInfo() override;

    std::string fileExtensionFormat() const;
    void doImport();
    void writeToDatabase(const persistence::Selector& selector);
};
}

#endif
