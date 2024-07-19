#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_JSON_EXPORTER_IMPORTER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_JSON_EXPORTER_IMPORTER_H

#include "src/persistence/exporterImporter/IExporterImporter.h"

#include "nlohmann/json.hpp"

#include <fstream>
#include <memory>

namespace persistence {
class JsonExporter: public IExporter {
public:
    JsonExporter();

    virtual util::Expected<void> writeToFile(const std::string& file, bool overwrite) override final;
    virtual void insert(const Data& info) override;
    virtual void insert(Data&& info) override;

    auto getJson() const noexcept { return json; }

private:
    virtual util::Expected<std::fstream> openFile(const std::string& file, bool overwrite);
    virtual void toStream(std::fstream stream, const nlohmann::json& json);

    nlohmann::json json;
    std::fstream stream;
};

class JsonImporter: public IImporter {
public:
    virtual util::Generator<util::Expected<Data>> loadFromFile(const std::string file) override final;

private:
    virtual util::Expected<std::fstream> openFile(const std::string& file);

    virtual nlohmann::json parse(std::fstream stream);
};
}

#endif
