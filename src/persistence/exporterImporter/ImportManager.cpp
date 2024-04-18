#include "src/persistence/exporterImporter/ImportManager.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

#include <filesystem>

namespace persistence {
constexpr auto COMPLETE = 1.0f;
constexpr int PERIOD_INDEX = 1;

std::future<tl::expected<void, Error>> 
ImportManager::doImport(const std::string& file)
{
    return std::async(
            std::launch::async,
            [this, 
             file]() -> tl::expected<void, Error>
            {
                // remove "."
                const auto format = std::filesystem::u8path(file).extension().string().substr(PERIOD_INDEX);
                if (auto ret = ExporterImporterFactory::getInstance().createImporter(format); ret) {
                    auto importer = std::move(ret.value());
                    auto loader = importer->loadFromFile(file);

                    while (loader.next()) {
                        if (const auto& ret = loader.getValue(); ret) {
                            this->cache.emplace(std::make_pair(ret.value().year, std::make_shared<const Data>(ret.value())));
                            this->yearImported++;
                        } else {
                            return tl::unexpected{ret.error()};
                        }
                    }
                } else {
                    return tl::unexpected{ret.error()};
                }

                return SUCCESS;
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

std::shared_ptr<const Data> ImportManager::find(int year) const
{
    if (cache.contains(year)) {
        return cache.at(year);
    }

    return nullptr;
}

std::optional<int> ImportManager::nextYear(int year) const
{
    if (auto it = cache.upper_bound(year); it != cache.end()) {
        return it->first;
    }

    return std::nullopt;
}

std::optional<int> ImportManager::previousYear(int year) const
{
    if (auto it = cache.lower_bound(year); it != cache.end() && it != cache.begin()) {
        return (--it)->first;
    }

    return std::nullopt;
}

std::optional<int> ImportManager::firstYear() const
{
    if (cache.empty()) {
        return std::nullopt;
    }

    return cache.cbegin()->first;
}
}