#ifndef SRC_TILE
#define SRC_TILE

#include <vector>
#include <cstddef>

#include <GL/gl.h>

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
    std::vector<std::byte> rgbaBlob;

    void glLoad();
    void stbLoad();
};

#endif /* SRC_TILE */