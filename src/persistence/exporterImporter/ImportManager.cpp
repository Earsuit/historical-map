#include "src/persistence/exporterImporter/ImportManager.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

namespace persistence {
constexpr auto COMPLETE = 1.0f;

std::future<tl::expected<void, Error>> ImportManager::doImport(Selector& selector,
                                                               const std::string& file, 
                                                               const std::string& format)
{
    return std::async(
            std::launch::async,
            [this, 
             file, 
             format, 
             &selector]() -> tl::expected<void, Error>
            {
                if (auto ret = ExporterImporterFactory::getInstance().createImporter(format); ret) {
                    auto importer = std::move(ret.value());

                    if (auto success = importer->loadFromFile(file); success) {
                        while(!importer->empty()) {
                            auto info = std::make_shared<const Data>(importer->front());
                            auto total = importer->size();
                            float count = 0;

                            for (const auto& country: info->countries) {
                                selector.select(country, info);
                            }

                            for (const auto& city : info->cities) {
                                selector.select(city, info);
                            }

                            selector.select(info->note, info);

                            importer->pop();

                            this->progress = (++count) / total;
                        }

                        this->progress = COMPLETE;
                    } else {
                        return tl::unexpected{success.error()};
                    }

                    return SUCCESS;
                } else {
                    return tl::unexpected{ret.error()};
                }
            }
        );
}

std::vector<std::string> ImportManager::supportedFormat()
{
    return persistence::ExporterImporterFactory::getInstance().supportedFormat();
}

float ImportManager::getProgress()
{
    return progress;
}
}