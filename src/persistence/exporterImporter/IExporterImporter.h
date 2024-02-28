#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_IEXPORTER_IMPORTER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_IEXPORTER_IMPORTER_H

#include "src/persistence/Data.h"
#include "src/persistence/exporterImporter/Util.h"

#include "tl/expected.hpp"

#include <string>
#include <set>

namespace persistence {
class IExporterImporter {
private:
    struct DataCompare
    {
        bool operator()(const auto& lhs, const auto& rhs) const
        {
            return lhs.year < rhs.year;
        }
    };

protected:
    std::set<Data, DataCompare> infos;
};

class IExporter: public IExporterImporter {
public:
    virtual ~IExporter() = default;
    virtual void insert(const Data& info) = 0;
    virtual void insert(Data&& info) = 0;
    virtual tl::expected<void, Error> writeToFile(const std::string& file, bool overwrite) = 0;
};

class IImporter: public IExporterImporter {
public:
    virtual ~IImporter() = default;
    virtual const Data& front() const noexcept = 0;
    virtual void pop() = 0;
    virtual bool empty() const noexcept = 0;
    virtual tl::expected<void, Error> loadFromFile(const std::string& file) = 0;
};
}

#endif
