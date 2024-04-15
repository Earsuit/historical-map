#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_UTIL_H
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_UTIL_H

#include "tl/expected.hpp"

#include <string>

namespace persistence {
enum class ErrorCode {
    FILE_EXISTS,
    FILE_NOT_EXISTS,
    PARSE_FILE_ERROR,
    FILE_FORMAT_NOT_SUPPORT,
};

struct Error {
    ErrorCode code;
    std::string msg;
};

static const auto SUCCESS = tl::expected<void, Error>{};
}

#endif
