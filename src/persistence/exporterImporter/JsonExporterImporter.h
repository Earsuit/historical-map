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

    virtual tl::expected<void, Error> writeToFile(const std::string& file, bool overwrite) override final;
    virtual void insert(const Data& info) override;
    virtual void insert(Data&& info) override;

    auto getJson() const noexcept { return json; }

private:
    virtual tl::expected<std::fstream, Error> openFile(const std::string& file, bool overwrite);
    virtual void toStream(std::fstream stream, const nlohmann::json& json);

    nlohmann::json json;
    std::fstream stream;
};

class JsonImporter: public IImporter {
public:
    virtual tl::expected<void, Error> open(const std::string& file) override final;
    virtual util::Generator<tl::expected<Data, Error>> load() override final;
    virtual std::optional<size_t> getSize() const noexcept override final;

private:
    virtual tl::expected<std::fstream, Error> openFile(const std::string& file);
    virtual nlohmann::json parse(std::fstream stream);

    std::optional<size_t> size;
    nlohmann::json json;
};
}

#endif
