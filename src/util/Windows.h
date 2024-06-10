// header contains necessary setup for windows

#define NOMINMAX    // disable the marcos of min/max from windows.h, we will use from algorithm
#include <windows.h>    // otherwise will get compilation errors on win
#define PATH_MAX MAX_PATH   // The PATH_MAX on windows is called MAX_PATH