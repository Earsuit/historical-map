#ifndef SRC_MODEL_EXPORT_MODEL_H
#define SRC_MODEL_EXPORT_MODEL_H

#include "src/persistence/exporterImporter/IExporterImporter.h"
#include "src/persistence/exporterImporter/ExporterImporterFactory.h"
#include "src/util/Error.h"

#include <string>
#include <vector>
#include <memory>

namespace model {
class ExportModel {
public:
    ExportModel();

    std::vector<std::string> getSupportedFormat() const;
    tl::expected<void, util::Error> setFormat(const std::string& format);
    void insert(const persistence::Data& info);
    tl::expected<void, util::Error> writeToFile(const std::string& path, bool overwrite);

    ExportModel(ExportModel&&) = delete;
    ExportModel(const ExportModel&) = delete;
    ExportModel& operator=(const ExportModel&) = delete;

private:
    persistence::ExporterImporterFactory& factory;
    std::unique_ptr<persistence::IExporter> exporter;
};
}

#endif
