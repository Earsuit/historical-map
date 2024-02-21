#ifndef SRC_PERSISTENCE_JSONFILEHANDLER
#define SRC_PERSISTENCE_JSONFILEHANDLER

#include "src/persistence/Data.h"

#include "tl/expected.hpp"

#include <set>
#include <memory>
#include <fstream>
#include <optional>

namespace persistence {
enum class Mode {
    Write,
    Read,
    OverWrite
};

enum class Error {
    FILE_EXISTS,
    FILE_NOT_EXISTS,
    INVALID_MODE
};

struct DataCompare
{
    bool operator()(const auto& lhs, const auto& rhs) const
    {
        return lhs.year < rhs.year;
    }
};

class JsonFileHandler {
public:
    static tl::expected<std::unique_ptr<JsonFileHandler>, Error> create(const std::string& file, Mode mode);

    ~JsonFileHandler();

    void setAuthor(const std::string& author);
    std::optional<Data> next();
    void add(Data data);
    void add(Data&& data);

private:
    JsonFileHandler(std::fstream&& stream, Mode mode);

    Mode mode;
    std::fstream stream;
    std::set<Data, DataCompare> infos;
    std::string author;
    decltype(infos.cbegin()) it;
};
}

#endif /* SRC_PERSISTENCE_JSONFILEHANDLER */
