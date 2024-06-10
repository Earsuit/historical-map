#include "Tile.h"

namespace tile {
Tile::Tile(const Coordinate& coord, const TileEngine::Image& image):
    coord{coord},
    image{image}
{
    glLoad();
}

Tile::Tile(const Coordinate& coord, TileEngine::Image&& image):
    coord{coord},
    image{std::move(image)}
{
    glLoad();
}

const Coordinate Tile::getCoordinate() const noexcept
{
    return coord;
}

void Tile::glLoad()
{
    const auto& [rgbBlob, width, height, channels] = image;

    if (rgbBlob.empty()) {
        return;
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, rgbBlob.data());
}

void* Tile::getTexture()
{
    // we will have warning C4312 on Win when dealing with 32-bit integers and 64-bit pointers
    return reinterpret_cast<void*>(static_cast<uintptr_t>(id));
}

bool Tile::operator==(const Tile& other) const noexcept
{
    return coord == other.coord;
}

}