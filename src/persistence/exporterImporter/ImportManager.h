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

namespace persistence {
class ImportManager {
public:
    std::future<tl::expected<std::map<int, std::shared_ptr<const Data>>, Error>> 
    doImport(const std::string& file);
    std::vector<std::string> supportedFormat() const;
    size_t numOfYearsImported() const noexcept;

private:
    std::atomic<size_t> yearImported = 0;
};
}

#endif
