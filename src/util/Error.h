#ifndef SRC_UTIL_ERROR_H
#define SRC_UTIL_ERROR_H

#include "tl/expected.hpp"

#include <string>

namespace util {
enum class ErrorCode {
    FILE_EXISTS,
    FILE_NOT_EXISTS,
    PARSE_FILE_ERROR,
    FILE_FORMAT_NOT_SUPPORT,
    FILE_EMPTY,
    INVALID_PARAM,
    OPERATION_CANCELED,
    NETWORK_ERROR,
};

struct Error {
    ErrorCode code;
    std::string msg;
};

static const auto SUCCESS = tl::expected<void, Error>{};

template<typename T>
using Expected = tl::expected<T, Error>;
using Unexpected = tl::unexpected<Error>;
}

#endif
