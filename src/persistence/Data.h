#ifndef SRC_PERSISTENCE_DATA_H
#define SRC_PERSISTENCE_DATA_H

#include <cereal/archives/binary.hpp>
#include <cereal/types/list.hpp>

#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <list>
#include <optional>
#include <cmath>

namespace persistence {
struct Coordinate {
    float latitude = 0;
    float longitude = 0;

    static constexpr float EPSILON = 1e-2;

    auto operator<=>(const Coordinate& other) const {
        if (std::fabs(latitude - other.latitude) < EPSILON &&
            std::fabs(longitude - other.longitude) < EPSILON) {
            return std::partial_ordering::equivalent;
        } else if (latitude < other.latitude || 
                  (std::fabs(latitude - other.latitude) < EPSILON && longitude < other.longitude)) {
            return std::partial_ordering::less;
        } else {
            return std::partial_ordering::greater;
        }
    }

    bool operator==(const Coordinate& other) const {
        return std::fabs(latitude - other.latitude) < EPSILON &&
               std::fabs(longitude - other.longitude) < EPSILON;
    }

    template <class Archive>
    void serialize(Archive& ar) {
        ar(latitude, longitude);
    }
};

struct Country {
    std::string name;
    std::list<Coordinate> borderContour;

    auto operator<=>(const Country&) const = default;
};

struct City {
    std::string name;
    Coordinate coordinate;

    auto operator<=>(const City&) const = default;
};

struct Note {
    std::string text;

    auto operator<=>(const Note&) const = default;
};

struct Data {
    int year = 0;
    std::list<Country> countries;
    std::list<City> cities;
    std::optional<Note> note;

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
        return std::vector<uint8_t>{view().cbegin(), view().cend()};
    }

    operator std::string() const 
    {
        return str();
    }
};

template<typename T>
T serializeContour(const std::list<Coordinate>& contour)
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
    std::list<Coordinate> contour;
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(contour); // Deserialize data into deserializedBorderContour
    }

    return contour;
}

}

#endif
