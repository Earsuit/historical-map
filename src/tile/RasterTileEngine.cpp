#include "src/tile/RasterTileEngine.h"

#include "external/stb/stb_image.h"

namespace tile {
RasterTileEngine::Image RasterTileEngine::toImage(const std::vector<std::byte>& rawBlob)
{
    if (rawBlob.empty()) {
        return {rawBlob, 0, 0, 0};
    }

    int width, height, channels;
    std::vector<std::byte> rgbBlob;

    stbi_set_flip_vertically_on_load(true);
    const auto ptr = reinterpret_cast<std::byte *>(stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(rawBlob.data()),
                                                                         static_cast<int>(rawBlob.size()), 
                                                                         &width, 
                                                                         &height,
                                                                         &channels, 
                                                                         STBI_rgb_alpha));
    
    if (ptr) {
        const auto size = width * height * STBI_rgb_alpha;
        rgbBlob.reserve(size);
        rgbBlob.insert(rgbBlob.cbegin(), ptr, ptr + size);
        stbi_image_free(ptr);
    }

    return {rgbBlob, width, height, channels};
}
}