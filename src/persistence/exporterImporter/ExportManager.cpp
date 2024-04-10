#include "src/persistence/exporterImporter/ExportManager.h"
#include "src/persistence/exporterImporter/IExporterImporter.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

#include <thread>
#include <chrono>

namespace persistence {
std::future<tl::expected<void, Error>> ExportManager::doExport(const Selector& selector,
                                                               const std::string& file, 
                                                               const std::string& format, 
                                                               bool overwrite)
{
    return std::async(
            std::launch::async,
            [this, 
             file, 
             format, 
             overwrite, 
             selections = std::move(selector.getSelections()),
             quantity = selector.getQuantity()]() mutable -> tl::expected<void, Error>
            {
                if (auto ret = ExporterImporterFactory::getInstance().createExporter(format); ret) {
                    auto exporter = std::move(ret.value());

                    this->progress = 0;
                    float count = 1;
                    while (selections.next()) {
                        exporter->insert(selections.getValue());
                        this->progress = (++count)/quantity;
                    }

                    this->progress = 1.0f;
                    return exporter->writeToFile(file, overwrite);
                } else {
                    return tl::unexpected{ret.error()};
                }
            }
        );
}

std::vector<std::string> ExportManager::supportedFormat()
{
    return persistence::ExporterImporterFactory::getInstance().supportedFormat();
}

float ExportManager::getExportProgress()
{
    return progress;
}
}