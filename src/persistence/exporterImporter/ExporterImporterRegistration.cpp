#include "src/persistence/exporterImporter/ExporterImporterRegistrar.h"
#include "src/persistence/exporterImporter/JsonExporterImporter.h"
#include "src/persistence/exporterImporter/BsonExporterImporter.h"

namespace persistence {
static ExporterImporterRegistrar<JsonImporter, JsonExporter> jsonExporterImporterRegistrar{"json"};
static ExporterImporterRegistrar<BsonImporter, BsonExporter> bsonExporterImporterRegistrar{"bson"};
}