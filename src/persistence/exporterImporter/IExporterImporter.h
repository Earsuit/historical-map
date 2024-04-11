#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_IEXPORTER_IMPORTER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_IEXPORTER_IMPORTER_H

#include "src/persistence/Data.h"
#include "src/persistence/exporterImporter/Util.h"

#include "tl/expected.hpp"

#include <string>
#include <set>
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
    struct CompareYear
    {
        bool operator()(const auto& lhs, const auto& rhs) const
        {
            return lhs.year < rhs.year;
        }
    };

    virtual ~IImporter() = default;
    
    const Data& front() const noexcept;
    void pop();
    bool empty() const noexcept;
    size_t size() const noexcept;
    tl::expected<void, Error> loadFromFile(const std::string& file);

private:
    // this cache is necessary because we have to parse the file to see if the content is valid or not
    std::set<Data, CompareYear> infos;

    virtual tl::expected<void, Error> loadTo(std::fstream stream, std::set<Data, CompareYear>& infos) = 0;
    virtual tl::expected<std::fstream, Error> openFile(const std::string& file) = 0;
};
}

#endif
