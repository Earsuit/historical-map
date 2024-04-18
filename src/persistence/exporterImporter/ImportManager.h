#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_IMPORT_MANAGER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_IMPORT_MANAGER_H

#include "src/persistence/Data.h"
#include "src/persistence/exporterImporter/Util.h"

#include "tl/expected.hpp"

#include <vector>
#include <string>
#include <future>
#include <map>
#include <memory>
#include <optional>

namespace persistence {
class ImportManager {
public:
    std::future<tl::expected<void, Error>> 
    doImport(const std::string& file);
    std::vector<std::string> supportedFormat() const;
    size_t numOfYearsImported() const noexcept;
    std::shared_ptr<const Data> find(int year) const;
    std::optional<int> nextYear(int year) const;
    std::optional<int> previousYear(int year) const;
    std::optional<int> firstYear() const;

private:
    std::atomic<size_t> yearImported = 0;
    std::map<int, std::shared_ptr<const Data>> cache;
};
}

#endif
