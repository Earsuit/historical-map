#ifndef SRC_TILE_TILE_H
#define SRC_TILE_TILE_H

#include "src/tile/Util.h"
#include "src/tile/TileEngine.h"

#include <vector>
#include <cstddef>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace tile {

class Tile {
public:
    Tile(const Coordinate& coord, const TileEngine::Image& image);
    Tile(const Coordinate& coord, TileEngine::Image&& image);

    void* getTexture();
    const Coordinate getCoordinate() const noexcept;

    bool operator==(const Tile& other) const noexcept;

private:
    Coordinate coord;
    GLuint id = 0;
    TileEngine::Image image;

    void glLoad();
};

}



#endif
