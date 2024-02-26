#include "src/persistence/exporterImporter/ExporterImporterRegistrar.h"
#include "src/persistence/exporterImporter/JsonExporterImporter.h"

namespace persistence {
static ExporterImporterRegistrar<JsonImporter, JsonExporter> jsonExporterImporterRegistrar{"JSON"};
}