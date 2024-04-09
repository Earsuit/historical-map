#include "src/persistence/exporterImporter/ExportManager.h"
#include "src/persistence/exporterImporter/IExporterImporter.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"

#include <thread>
#include <chrono>

namespace persistence {
void ExportManager::select(const Country& country, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        if (selections[from->year].countries.insert(country.name).second) {
            itemQuantity++;
        }
    } else {
        selections.emplace(std::make_pair(from->year, Selected{.from = from, .countries = {country.name}}));
        itemQuantity++;
    }
}

void ExportManager::select(const City& city, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        if(selections[from->year].cities.insert(city.name).second) {
            itemQuantity++;
        }
    } else {
        selections.emplace(std::make_pair(from->year, Selected{.from = from, .cities={city.name}}));
        itemQuantity++;
    }
}

void ExportManager::select(const Note& note, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        if (!selections[from->year].note) {
            selections[from->year].note = true;
            itemQuantity++;
        }
    } else {
        selections.emplace(std::make_pair(from->year, Selected{.from = from, .note=true}));
        itemQuantity++;
    }
}

void ExportManager::deselect(const Country& country, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        if (selections[from->year].countries.contains(country.name)) {
            selections[from->year].countries.erase(country.name);
            itemQuantity--;
        }
    }
}

void ExportManager::deselect(const City& city, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        if (selections[from->year].cities.contains(city.name)) {
            selections[from->year].cities.erase(city.name);
            itemQuantity--;
        }
    }
}

void ExportManager::deselect(const Note& note, std::shared_ptr<const Data> from)
{
    if (selections.contains(from->year)) {
        if (selections[from->year].note) {
            selections[from->year].note = false;
            itemQuantity--;
        }
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

                    this->progress = 0;
                    for (const auto& [year, selected] : this->selections) {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        if (!selected.empty()) {
                            Data toBeExported{.year = year};

                            for (auto& country : selected.from->countries) {
                                if (selected.countries.contains(country.name)) {
                                    toBeExported.countries.emplace_back(country);
                                    this->progress = this->progress + 1;
                                }
                            }

                            for (auto& city : selected.from->cities) {
                                if (selected.cities.contains(city.name)) {
                                    toBeExported.cities.emplace_back(city);
                                    this->progress = this->progress + 1;
                                }
                            }

                            if (selected.note) {
                                toBeExported.note = selected.from->note;
                                this->progress = this->progress + 1;
                            }

                            exporter->insert(std::move(toBeExported));
                        }
                    }

                    this->progress = this->itemQuantity;
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

float ExportManager::getExportProgress()
{
    return progress / itemQuantity;
}
}