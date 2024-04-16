#include "src/persistence/exporterImporter/ImportManager.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

#include <filesystem>

namespace persistence {
constexpr auto COMPLETE = 1.0f;

std::future<tl::expected<std::map<int, std::shared_ptr<const Data>>, Error>> 
ImportManager::doImport(const std::string& file)
{
    return std::async(
            std::launch::async,
            [this, 
             file]() -> tl::expected<std::map<int, std::shared_ptr<const Data>>, Error>
            {
                const auto format = std::filesystem::u8path(file).extension();
                if (auto ret = ExporterImporterFactory::getInstance().createImporter(format); ret) {
                    auto importer = std::move(ret.value());
                    auto loader = importer->loadFromFile(file);
                    std::map<int, std::shared_ptr<const Data>> infos;

                    while (loader.next()) {
                        if (const auto& ret = loader.getValue(); ret) {
                            infos.emplace(std::make_pair(ret.value().year, std::make_shared<const Data>(ret.value())));
                            this->yearImported++;
                        } else {
                            return tl::unexpected{ret.error()};
                        }
                    }

                    return infos;
                } else {
                    return tl::unexpected{ret.error()};
                }
            }
        );
}

std::vector<std::string> ImportManager::supportedFormat() const
{
    return persistence::ExporterImporterFactory::getInstance().supportedFormat();
}

size_t ImportManager::numOfYearsImported() const noexcept
{
    return yearImported;
}
}