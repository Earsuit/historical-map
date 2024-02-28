#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_BSON_EXPORTER_IMPORTER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_BSON_EXPORTER_IMPORTER_H

#include "src/persistence/exporterImporter/JsonExporterImporter.h"

#include <fstream>

namespace persistence {
class BsonExporter: public JsonExporter {
public:
    tl::expected<void, Error> writeToFile(const std::string& file, bool overwrite) override;

private:
    tl::expected<void, Error> openFile(const std::string& file, bool overwrite);

    std::fstream stream;
};

class BsonImporter: public JsonImporter {
public:
    tl::expected<void, Error> loadFromFile(const std::string& file) override;

private:
    tl::expected<void, Error> openFile(const std::string& file);

    std::fstream stream;
};
}

#endif
