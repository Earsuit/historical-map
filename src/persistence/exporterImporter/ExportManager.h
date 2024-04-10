#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORT_MANAGER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORT_MANAGER_H

#include "src/persistence/Data.h"
#include "src/persistence/exporterImporter/Util.h"
#include "src/persistence/Selector.h"

#include "tl/expected.hpp"

#include <set>
#include <string>
#include <memory>
#include <map>
#include <future>
#include <vector>
#include <atomic>

namespace persistence {
class ExportManager {
public:
    std::future<tl::expected<void, Error>> doExport(const Selector& selector,
                                                    const std::string& file, 
                                                    const std::string& format, 
                                                    bool overwrite);
    std::vector<std::string> supportedFormat();
    float getExportProgress();

private:
    std::atomic<float> progress = 0;
};
}

#endif
