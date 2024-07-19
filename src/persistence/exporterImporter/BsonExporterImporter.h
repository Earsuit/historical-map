#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_BSON_EXPORTER_IMPORTER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_BSON_EXPORTER_IMPORTER_H

#include "src/persistence/exporterImporter/JsonExporterImporter.h"
#include "src/util/Error.h"

#include <fstream>

namespace persistence {
class BsonExporter: public JsonExporter {
private:
    virtual util::Expected<std::fstream> openFile(const std::string& file, bool overwrite) override;
    virtual void toStream(std::fstream stream, const nlohmann::json& json) override;

    std::fstream stream;
};

class BsonImporter: public JsonImporter {
private:
    virtual nlohmann::json parse(std::fstream stream) override;
    virtual util::Expected<std::fstream> openFile(const std::string& file) override;
};
}

#endif
