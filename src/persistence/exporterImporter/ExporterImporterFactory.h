#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORTER_IMPORTER_FACTORY_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORTER_IMPORTER_FACTORY_H

#include "src/persistence/exporterImporter/IExporterImporter.h"
#include "src/persistence/exporterImporter/Util.h"

#include "tl/expected.hpp"

#include <string>
#include <map>
#include <functional>

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

    template<typename T>
    void registerImporter(const std::string& name, Builder<T> builder) {
        builderUniqueCheck(importers, name);
        importers[name] = builder;
    }

    template<typename T>
    void registerExporter(const std::string& name, Builder<T> builder) {
        builderUniqueCheck(exporters, name);
        exporters[name] = builder;
    }

    ExporterImporterFactory(const ExporterImporterFactory&) = delete;
    ExporterImporterFactory& operator=(const ExporterImporterFactory&) = delete;

    static ExporterImporterFactory& getInstance() {
        static ExporterImporterFactory factory{};
        return factory;
    }

private:
    ExporterImporterFactory() = default;

    template<typename T>
    tl::expected<std::unique_ptr<T>, Error> create(std::map<std::string, Builder<T>>& builders, 
                  const std::string& name) {
        if (builders.contains(name)) {
            return builders[name]();
        } else {
            return tl::unexpected(Error{ErrorCode::FILE_FORMAT_NOT_SUPPORT});
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
