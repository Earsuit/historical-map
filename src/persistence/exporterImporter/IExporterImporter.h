#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_IEXPORTER_IMPORTER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_IEXPORTER_IMPORTER_H

#include "src/persistence/Data.h"
#include "src/persistence/exporterImporter/Util.h"
#include "src/util/Generator.h"

#include "tl/expected.hpp"

#include <string>
#include <fstream>
#include <optional>

namespace persistence {
class IExporter {
public:
    virtual ~IExporter() = default;
    virtual void insert(const Data& info) = 0;
    virtual void insert(Data&& info) = 0;
    virtual tl::expected<void, Error> writeToFile(const std::string& file, bool overwrite) = 0;
};

class IImporter {
public:
    virtual ~IImporter() = default;

    virtual tl::expected<void, Error> open(const std::string& file) = 0;
    virtual util::Generator<tl::expected<Data, Error>> load() = 0;
    // some format might not be able to extract size
    virtual std::optional<size_t> getSize() const noexcept = 0;
};
}

#endif
