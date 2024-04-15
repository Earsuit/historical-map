#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_IEXPORTER_IMPORTER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_IEXPORTER_IMPORTER_H

#include "src/persistence/Data.h"
#include "src/persistence/exporterImporter/Util.h"
#include "src/util/Generator.h"

#include "tl/expected.hpp"

#include <string>
#include <fstream>

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

    // It must taken a copy of the path since it returns a coroutine generator, 
    // if passing a temperatory object, it might be destroyed before using
    virtual util::Generator<tl::expected<Data, Error>> loadFromFile(const std::string file) = 0;
};
}

#endif
