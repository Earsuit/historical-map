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
             &selections = selector.getSelections(),
             quantity = selector.getQuantity()]() -> tl::expected<void, Error>
            {
                if (auto ret = ExporterImporterFactory::getInstance().createExporter(format); ret) {
                    auto exporter = std::move(ret.value());

                    this->progress = 0;
                    float count = 1;
                    for (const auto& [year, selected] : selections) {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        if (!selected.empty()) {
                            Data toBeExported{.year = year};

                            for (auto& country : selected.from->countries) {
                                if (selected.countries.contains(country.name)) {
                                    toBeExported.countries.emplace_back(country);
                                    this->progress = (++count)/quantity;
                                }
                            }

                            for (auto& city : selected.from->cities) {
                                if (selected.cities.contains(city.name)) {
                                    toBeExported.cities.emplace_back(city);
                                    this->progress = (++count)/quantity;
                                }
                            }

                            if (selected.note) {
                                toBeExported.note = selected.from->note;
                                this->progress = (++count)/quantity;
                            }

                            exporter->insert(std::move(toBeExported));
                        }
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