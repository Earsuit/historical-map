#ifndef SRC_MODEL_IMPORT_MODEL_H
#define SRC_MODEL_IMPORT_MODEL_H

#include "src/persistence/exporterImporter/IExporterImporter.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"
#include "src/persistence/Data.h"
#include "src/util/Error.h"

#include <string>
#include <vector>
#include <memory>

namespace model {
class ImportModel {
public:
    ImportModel();

    std::vector<std::string> getSupportedFormat() const;
    tl::expected<void, util::Error> setFormat(const std::string& format);
    util::Generator<tl::expected<persistence::Data, util::Error>> loadFromFile(const std::string& file);

    ImportModel(ImportModel&&) = delete;
    ImportModel(const ImportModel&) = delete;
    ImportModel& operator=(const ImportModel&) = delete;

private:
    persistence::ExporterImporterFactory& factory;
    std::unique_ptr<persistence::IImporter> importer;
};
}

#endif