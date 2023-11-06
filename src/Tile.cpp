#include "Tile.h"

#include <stb_image.h>

Tile::Tile(int x, int y, int z, const std::vector<std::byte> &rawBlob):
    x{x},
    y{y},
    z{z},
    rawBlob{rawBlob}
{
}

void Tile::stbLoad() {
    stbi_set_flip_vertically_on_load(false);
    const auto ptr = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(rawBlob.data()),
                                static_cast<int>(rawBlob.size()), &width, &height,
                                &channels, STBI_rgb_alpha);
    if (ptr) {
        const size_t nbytes{size_t(width * height * STBI_rgb_alpha)};
        rgbaBlob.resize(nbytes);
        rgbaBlob.shrink_to_fit();
        const auto byteptr{reinterpret_cast<std::byte *>(ptr)};
        rgbaBlob.insert(rgbaBlob.begin(), byteptr, byteptr + nbytes);
        stbi_image_free(ptr);
    }
}

void Tile::glLoad()
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, rgbaBlob.data());
}