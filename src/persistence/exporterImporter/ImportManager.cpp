#include "src/persistence/exporterImporter/ImportManager.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

namespace persistence {
constexpr auto COMPLETE = 1.0f;

std::future<tl::expected<std::map<int, std::shared_ptr<const Data>>, Error>> 
ImportManager::doImport(const std::string& file, 
                        const std::string& format)
{
    return std::async(
            std::launch::async,
            [this, 
             file, 
             format]() -> tl::expected<std::map<int, std::shared_ptr<const Data>>, Error>
            {
                if (auto ret = ExporterImporterFactory::getInstance().createImporter(format); ret) {
                    auto importer = std::move(ret.value());

                    if (auto ret = importer->open(file); ret) {
                        auto total = importer->getSize();
                        float count = 0;
                        auto loader = importer->load();
                        std::map<int, std::shared_ptr<const Data>> infos;

                        while (loader.next()) {
                            if (const auto& ret = loader.getValue(); ret) {
                                infos.emplace(std::make_pair(ret.value().year, std::make_shared<const Data>(ret.value())));

                                if (total) {
                                    this->progress = (++count) / *total;
                                }
                            } else {
                                return tl::unexpected{ret.error()};
                            }
                        }

                        this->progress = COMPLETE;

                        return infos;
                    } else {
                        return tl::unexpected{ret.error()};
                    }
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