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
#include <CoreFoundation/CoreFoundation.h>
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

// returns executable path for non-mac os
inline std::filesystem::path getAppBundlePath()
{
#ifdef __APPLE__
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (mainBundle == NULL) {
        return "";
    }

    CFURLRef bundleURL = CFBundleCopyBundleURL(mainBundle);
    if (bundleURL == NULL) {
        return "";
    }

    // Convert the URL to a file system path
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(bundleURL, true, (UInt8 *)path, PATH_MAX)) {
        CFRelease(bundleURL);
        return "";
    }

    CFRelease(bundleURL);
    return path;
#else
    return getExecutablePath();
#endif
}
}

#endif
