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

struct Stream : public std::stringstream {
    using std::stringstream::stringstream;

    Stream(Stream&& ss) :
        std::stringstream(std::move(ss))
    {
    }

    Stream(const uint8_t* blob, size_t len) :
        std::stringstream(std::string{reinterpret_cast<const char*>(blob), len})
    {
    }

    operator std::vector<uint8_t>() const 
    {
        const auto string{str()};
        return std::vector<uint8_t>{string.cbegin(), string.cend()};
    }

    operator std::string() const 
    {
        return str();
    }
};

template<typename T>
T serializeContour(const std::vector<Coordinate>& contour)
{
    T ss;
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(contour); // Serialize the vector of Coordinates
    }

    return ss;
}

template<typename T>
auto deserializeContour(T&& ss)
{
    std::vector<Coordinate> contour;
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(contour); // Deserialize data into deserializedBorderContour
    }

    return contour;
}

}

#endif
