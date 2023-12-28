#ifndef SRC_PERSISTENCE_DATA_H
#define SRC_PERSISTENCE_DATA_H

#include <string>
#include <vector>
#include <optional>

namespace persistence {
struct Coordinate {
    float latitude = 0;
    float longitude = 0;

    auto operator<=>(const Coordinate&) const = default;
};

struct Country {
    std::string name;
    std::vector<Coordinate> borderContour;

    auto operator<=>(const Country&) const = default;
};

struct City {
    std::string name;
    Coordinate coordinate;

    auto operator<=>(const City&) const = default;
};

struct Event {
    std::string description;

    auto operator<=>(const Event&) const = default;
};

struct Data {
    int year = 0;
    std::vector<Country> countries;
    std::vector<City> cities;
    std::optional<Event> event;

    auto operator<=>(const Data&) const = default;
};
}

#endif
