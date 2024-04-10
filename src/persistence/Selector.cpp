#include "src/persistence/Selector.h"

namespace persistence {
void Selector::select(const Country& country, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        if (selection[from->year].countries.insert(country.name).second) {
            quantity++;
        }
    } else {
        selection.emplace(std::make_pair(from->year, Selected{.from = from, .countries = {country.name}}));
        quantity++;
    }
}

void Selector::select(const City& city, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        if(selection[from->year].cities.insert(city.name).second) {
            quantity++;
        }
    } else {
        selection.emplace(std::make_pair(from->year, Selected{.from = from, .cities={city.name}}));
        quantity++;
    }
}

void Selector::select(const Note& note, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        if (!selection[from->year].note) {
            selection[from->year].note = true;
            quantity++;
        }
    } else {
        selection.emplace(std::make_pair(from->year, Selected{.from = from, .note=true}));
        quantity++;
    }
}

void Selector::deselect(const Country& country, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        if (selection[from->year].countries.contains(country.name)) {
            selection[from->year].countries.erase(country.name);
            quantity--;
        }
    }
}

void Selector::deselect(const City& city, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        if (selection[from->year].cities.contains(city.name)) {
            selection[from->year].cities.erase(city.name);
            quantity--;
        }
    }
}

void Selector::deselect(const Note& note, std::shared_ptr<const Data> from)
{
    if (selection.contains(from->year)) {
        if (selection[from->year].note) {
            selection[from->year].note = false;
            quantity--;
        }
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
        quantity -= selection[year].countries.size();
        quantity -= selection[year].cities.size();
        quantity -= selection[year].note ? 1 : 0;

        selection.erase(year);
    }
}

void Selector::clearAll()
{
    selection.clear();
}
}