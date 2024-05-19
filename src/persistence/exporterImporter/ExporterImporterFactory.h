#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORTER_IMPORTER_FACTORY_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORTER_IMPORTER_FACTORY_H

#include "src/persistence/exporterImporter/IExporterImporter.h"
#include "src/util/Error.h"

#include "tl/expected.hpp"

#include <string>
#include <map>
#include <functional>
#include <vector>

namespace persistence {
class ExporterImporterFactory {
public:
    template<typename T>
    using Builder = std::function<std::unique_ptr<T>()>;

    auto createImporter(const std::string& name) {
        return create<IImporter>(importers, name);
    }

    auto createExporter(const std::string& name)
    {
        return create<IExporter>(exporters, name);
    }

    void registerImporter(const std::string& name, Builder<IImporter> builder) {
        builderUniqueCheck(importers, name);
        importers[name] = builder;
    }

    void registerExporter(const std::string& name, Builder<IExporter> builder) {
        builderUniqueCheck(exporters, name);
        exporters[name] = builder;
    }

    ExporterImporterFactory(const ExporterImporterFactory&) = delete;
    ExporterImporterFactory& operator=(const ExporterImporterFactory&) = delete;

    static ExporterImporterFactory& getInstance() {
        static ExporterImporterFactory factory{};
        return factory;
    }

    std::vector<std::string> supportedFormat() const
    {
        std::vector<std::string> formats;
        for (const auto& [format, builder] : importers) {
            formats.emplace_back(format);
        }

        return formats;
    }

private:
    ExporterImporterFactory() = default;

    template<typename T>
    tl::expected<std::unique_ptr<T>, util::Error> create(std::map<std::string, Builder<T>>& builders, 
                  const std::string& name) {
        if (builders.contains(name)) {
            return builders[name]();
        } else {
            return tl::unexpected(util::Error{util::ErrorCode::FILE_FORMAT_NOT_SUPPORT});
        }
    }

    void builderUniqueCheck(const auto& builders, const std::string& name) {
        if (builders.contains(name)) {
            throw std::runtime_error(name + " builder already exist!");
        }
    }

    std::map<std::string, Builder<IImporter>> importers;
    std::map<std::string, Builder<IExporter>> exporters;
};
}

#endif
