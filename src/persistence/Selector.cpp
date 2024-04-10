#include "src/persistence/Selector.h"

namespace persistence {
void Selector::select(const Country& country, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        selection[from->year].countries.insert(country.name);
    } else {
        selection.emplace(std::make_pair(from->year, Selected{.from = from, .countries = {country.name}}));
    }
}

void Selector::select(const City& city, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        selection[from->year].cities.insert(city.name);
    } else {
        selection.emplace(std::make_pair(from->year, Selected{.from = from, .cities={city.name}}));
    }
}

void Selector::select(const Note& note, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        selection[from->year].note = true;
    } else {
        selection.emplace(std::make_pair(from->year, Selected{.from = from, .note=true}));
    }
}

void Selector::deselect(const Country& country, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        if (selection[from->year].countries.contains(country.name)) {
            selection[from->year].countries.erase(country.name);
        }
    }
}

void Selector::deselect(const City& city, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        if (selection[from->year].cities.contains(city.name)) {
            selection[from->year].cities.erase(city.name);
        }
    }
}

void Selector::deselect(const Note& note, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        selection[from->year].note = false;
    }
}

bool Selector::isSelected(const Country& country, int year)
{
    if (selection.contains(year)) {
        return selection[year].countries.contains(country.name);
    } else {
        return false;
    }
}

bool Selector::isSelected(const City& city, int year)
{
    if (selection.contains(year)) {
        return selection[year].cities.contains(city.name);
    } else {
        return false;
    }
}

bool Selector::isSelected(const Note& note, int year)
{
    if (selection.contains(year)) {
        return selection[year].note;
    } else {
        return false;
    }
}

void Selector::clear(int year)
{
    if (selection.contains(year)) {
        selection.erase(year);
    }
}

void Selector::clearAll()
{
    selection.clear();
}

util::Generator<Data> Selector::getSelections() const
{
    for (const auto& [year, selected] : selection) {
        if (selected.empty()) {
            continue;
        }
        
        Data out{.year = year};
        
        for (auto& country : selected.from->countries) {
            if (selected.countries.contains(country.name)) {
                out.countries.emplace_back(country);
            }
        }

        for (auto& city : selected.from->cities) {
            if (selected.cities.contains(city.name)) {
                out.cities.emplace_back(city);
            }
        }

        if (selected.note) {
            out.note = selected.from->note;
        }

        co_yield out;
    }
}
}