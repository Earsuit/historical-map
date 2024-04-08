#include "src/persistence/exporterImporter/ExportManager.h"
#include "src/persistence/exporterImporter/IExporterImporter.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

namespace persistence {
void ExportManager::select(const Country& country, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        selections[from->year].countries.insert(country.name);
    } else {
        selections.emplace(std::make_pair(from->year, Selected{.from = from, .countries = {country.name}}));
    }
}

void ExportManager::select(const City& city, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        selections[from->year].cities.insert(city.name);
    } else {
        selections.emplace(std::make_pair(from->year, Selected{.from = from, .cities={city.name}}));
    }
}

void ExportManager::select(const Note& note, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        selections[from->year].note = true;
    } else {
        selections.emplace(std::make_pair(from->year, Selected{.from = from, .note=true}));
    }
}

void ExportManager::deselect(const Country& country, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        if (selections[from->year].countries.contains(country.name)) {
            selections[from->year].countries.erase(country.name);
        }
    }
}

void ExportManager::deselect(const City& city, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        if (selections[from->year].cities.contains(city.name)) {
            selections[from->year].cities.erase(city.name);
        }
    }
}

void ExportManager::deselect(const Note& note, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        selections[from->year].note = false;
    }
}

std::future<tl::expected<void, Error>> ExportManager::doExport(const std::string& file, const std::string& format, bool overwrite)
{
    return std::async(
            std::launch::async,
            [this, file, format, overwrite]() -> tl::expected<void, Error>
            {
                if (auto ret = ExporterImporterFactory::getInstance().createExporter(format); ret) {
                    auto exporter = std::move(ret.value());

                    for (const auto& [year, selected] : this->selections) {
                        if (!selected.empty()) {
                            Data toBeExported{.year = year};

                            for (auto& country : selected.from->countries) {
                                if (selected.countries.contains(country.name)) {
                                    toBeExported.countries.emplace_back(country);
                                }
                            }

                            for (auto& city : selected.from->cities) {
                                if (selected.cities.contains(city.name)) {
                                    toBeExported.cities.emplace_back(city);
                                }
                            }

                            if (selected.note) {
                                toBeExported.note = selected.from->note;
                            }

                            exporter->insert(std::move(toBeExported));
                        }
                    }

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

bool ExportManager::isSelected(const Country& country, int year)
{
    if (selections.contains(year)) {
        return selections[year].countries.contains(country.name);
    } else {
        return false;
    }
}

bool ExportManager::isSelected(const City& city, int year)
{
    if (selections.contains(year)) {
        return selections[year].cities.contains(city.name);
    } else {
        return false;
    }
}

bool ExportManager::isSelected(const Note& note, int year)
{
    if (selections.contains(year)) {
        return selections[year].note;
    } else {
        return false;
    }
}
}