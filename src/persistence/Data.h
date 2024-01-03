#ifndef SRC_PERSISTENCE_DATA_H
#define SRC_PERSISTENCE_DATA_H

#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include <cstdint>

namespace persistence {
struct Coordinate {
    double latitude = 0;
    double longitude = 0;

    auto operator<=>(const Coordinate&) const = default;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(latitude, longitude);
    }
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

static auto serializeContour(const std::vector<Coordinate>& contour)
{
    std::stringstream ss;
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(contour); // Serialize the vector of Coordinates
    }

    std::string serializedStr = ss.str();
    return std::vector<uint8_t>{serializedStr.begin(), serializedStr.end()};
}

static auto deserializeContour(const std::vector<uint8_t>& bytes)
{
    std::stringstream ss{std::string{reinterpret_cast<const char*>(bytes.data()), bytes.size()}};
    std::vector<Coordinate> contour;
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(contour); // Deserialize data into deserializedBorderContour
    }

    return contour;
}
}

#endif
