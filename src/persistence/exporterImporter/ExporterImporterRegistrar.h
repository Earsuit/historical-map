#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORTER_IMPORTER_REGISTRAR_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORTER_IMPORTER_REGISTRAR_H

#include "src/persistence/exporterImporter/ExporterImporterFactory.h"
#include "src/persistence/exporterImporter/Util.h"

#include <string>
#include <type_traits>

namespace persistence {
template<typename Importer, typename Exporter>
requires std::is_base_of_v<IImporter, Importer> && std::is_base_of_v<IExporter, Exporter>
class ExporterImporterRegistrar {
public:
    ExporterImporterRegistrar(const std::string& name)
    {
        ExporterImporterFactory::getInstance().registerImporter<Importer>(name, [](){
            return std::make_unique<Importer>();
        });
        ExporterImporterFactory::getInstance().registerExporter<Exporter>(name, [](){
            return std::make_unique<Exporter>();
        });
    }

    ExporterImporterRegistrar(const std::string& name, 
                              ExporterImporterFactory::Builder<Importer> importerBuilder, 
                              ExporterImporterFactory::Builder<Exporter> exporterBuilder)
    {
        ExporterImporterFactory::getInstance().registerImporter<Importer>(name, importerBuilder);
        ExporterImporterFactory::getInstance().registerExporter<Exporter>(name, exporterBuilder);
    }
};
}

#endif
