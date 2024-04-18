#ifndef SRC_PERSISTENCE_SELECTOR
#define SRC_PERSISTENCE_SELECTOR

#include "src/persistence/Data.h"
#include "src/util/Generator.h"

#include <memory>
#include <map>
#include <set>
#include <string>

namespace persistence {
class Selector {
public:
    void select(const Country& country, std::shared_ptr<const Data> from);
    void select(const City& city, std::shared_ptr<const Data> from);
    void select([[maybe_unused]] const Note& note, std::shared_ptr<const Data> from);
 
    void deselect(const Country& country, std::shared_ptr<const Data> from);
    void deselect(const City& city, std::shared_ptr<const Data> from);
    void deselect([[maybe_unused]] const Note& note, std::shared_ptr<const Data> from);
 
    bool isSelected(const Country& country, int year) const;
    bool isSelected(const City& city, int year) const;
    bool isSelected([[maybe_unused]] const Note& note, int year) const;

    void clear(int year);
    void clearAll();

    int getQuantity() const noexcept { return selection.size(); };

    util::Generator<std::shared_ptr<const Data>> getSelections() const;

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

    std::map<int, Selected> selection;
};
}

#endif /* SRC_PERSISTENCE_SELECTOR */
