#ifndef SRC_TILE_TILE_H
#define SRC_TILE_TILE_H

#include <vector>
#include <cstddef>

#include <GL/gl.h>

namespace tile {
class Tile {
public:
    Tile(int x, int y, int z, const std::vector<std::byte> &rawBlob);

    void* getTexture();

private:
    int x = 0;
    int y = 0;
    int z = 0;
    int channels;
    GLuint id = 0;
    GLsizei width = 0;
    GLsizei height = 0;
    std::vector<std::byte> rawBlob;

    void glLoad();
};

}



#endif
