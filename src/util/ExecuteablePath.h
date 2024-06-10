#ifndef SRC_UTIL_EXECUTEABLE_PATH_H
#define SRC_UTIL_EXECUTEABLE_PATH_H

#include <filesystem>
#include <limits>

#ifdef _WIN32
#include "src/util/Windows.h"
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace util {
inline std::filesystem::path getExecutablePath() {
    char buffer[PATH_MAX];

#ifdef _WIN32
    if (GetModuleFileName(NULL, buffer, MAX_PATH) != 0) {
        return std::filesystem::path{buffer};
    }
#elif defined(__linux__)
    const auto len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::filesystem::path{buffer};
    }
#elif defined(__APPLE__)
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return std::filesystem::path{buffer};
    }
#endif

    return std::filesystem::path{};
}
}

#endif
