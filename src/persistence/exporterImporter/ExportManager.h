#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORT_MANAGER_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_EXPORT_MANAGER_H

#include "src/persistence/Data.h"
#include "src/persistence/exporterImporter/Util.h"

#include "tl/expected.hpp"

#include <set>
#include <string>
#include <memory>
#include <map>
#include <future>
#include <vector>

namespace persistence {
class ExportManager {
public:
    void select(const Country& country, std::shared_ptr<const Data> from);
    void select(const City& city, std::shared_ptr<const Data> from);
    void select([[maybe_unused]] const Note& note, std::shared_ptr<const Data> from);

    void deselect(const Country& country, std::shared_ptr<const Data> from);
    void deselect(const City& city, std::shared_ptr<const Data> from);
    void deselect([[maybe_unused]] const Note& note, std::shared_ptr<const Data> from);

    bool isSelected(const Country& country, int year);
    bool isSelected(const City& city, int year);
    bool isSelected([[maybe_unused]] const Note& note, int year);

    std::future<tl::expected<void, Error>> doExport(const std::string& file, const std::string& format, bool overwrite);
    std::vector<std::string> supportedFormat();

private:
    struct CompareString
    {
        bool operator()(const std::string& lhs, const std::string& rhs) const noexcept
        {
            return std::hash<std::string>{}(lhs) < std::hash<std::string>{}(rhs);
        }
    };

    struct Selected
    {
        std::shared_ptr<const Data> from;
        std::set<std::string, CompareString> countries;
        std::set<std::string, CompareString> cities;
        bool note = false;

        bool empty() const noexcept
        {
            return countries.empty() && cities.empty() && note == false;
        }
    };

    std::map<int, Selected> selections;
};
}

#endif
