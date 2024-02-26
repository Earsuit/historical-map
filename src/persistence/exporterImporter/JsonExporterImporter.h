#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_JSON_EXPORTER_IMPORTER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_JSON_EXPORTER_IMPORTER_H

#include "src/persistence/exporterImporter/IExporterImporter.h"

#include "spdlog/spdlog.h"

#include <fstream>
#include <memory>

namespace persistence {
class JsonExporter: public IExporter {
public:
    tl::expected<void, Error> writeToFile(const std::string& file, bool overwrite) override;
    void insert(Data info) override;
    void insert(Data&& info) override;

private:
    tl::expected<void, Error> openFile(const std::string& file, bool overwrite);

    std::fstream stream;
};

class JsonImporter: public IImporter {
public:
    const Data& front() const noexcept override;
    void pop() override;
    bool empty() const noexcept override;
    tl::expected<void, Error> loadFromFile(const std::string& file) override;

private:
    tl::expected<void, Error> openFile(const std::string& file);

    std::fstream stream;
};
}

#endif
