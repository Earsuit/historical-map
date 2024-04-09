#ifndef SRC_UI_UTIL_H
#define SRC_UI_UTIL_H

namespace ui {
void helpMarker(const char* message);

// we are not so care about the error of floating comparison 
template<typename T>
bool inBound(T v, T min, T max)
{
    return (v > min) && (v < max);
}

void alignForWidth(float width, float alignment = 0.5f);
}

#endif
