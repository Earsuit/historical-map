#include "Tile.h"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

namespace tile {
Tile::Tile(const Coordinate& coord, const std::vector<std::byte> &rawBlob):
    coord{coord},
    rawBlob{rawBlob}
{
    
}

const Coordinate Tile::getCoordinate() const noexcept
{
    return coord;
}

void Tile::glLoad()
{
    if (rawBlob.empty()) {
        return;
    }

    stbi_set_flip_vertically_on_load(true);
    const auto ptr = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(rawBlob.data()),
                                static_cast<int>(rawBlob.size()), &width, &height,
                                &channels, STBI_rgb_alpha);
    if (ptr) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, ptr);
        stbi_image_free(ptr);
    }
}

void* Tile::getTexture()
{
    if (id == 0) {
        glLoad();
    }
    return reinterpret_cast<void*>(id);
}

bool Tile::operator==(const Tile& other) const noexcept
{
    return coord == other.coord;
}

}