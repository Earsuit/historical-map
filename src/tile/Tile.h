#ifndef SRC_TILE_TILE_H
#define SRC_TILE_TILE_H

#include "src/tile/Util.h"

#include <vector>
#include <cstddef>

#include <GL/gl.h>

namespace tile {

class Tile {
public:
    Tile(const Coordinate& coord, const std::vector<std::byte>& rawBlob);
    Tile(const Coordinate& coord, std::vector<std::byte>&& rawBlob);

    void* getTexture();
    const Coordinate getCoordinate() const noexcept;

    bool operator==(const Tile& other) const noexcept;

private:
    Coordinate coord;
    int channels;
    GLuint id = 0;
    GLsizei width = 0;
    GLsizei height = 0;
    std::vector<std::byte> rawBlob;

    void glLoad();
};

}



#endif
