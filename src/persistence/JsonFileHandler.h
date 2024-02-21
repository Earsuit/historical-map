#ifndef SRC_PERSISTENCE_JSONFILEHANDLER
#define SRC_PERSISTENCE_JSONFILEHANDLER

#include "src/persistence/Data.h"

#include "tl/expected.hpp"

#include <map>
#include <memory>
#include <fstream>

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

class JsonFileHandler {
public:
    static tl::expected<std::unique_ptr<JsonFileHandler>, Error> create(const std::string& file, Mode mode);

    ~JsonFileHandler();

    void setAuthor(const std::string& author);
    std::shared_ptr<Data> load(int year);
    void add(Data data);
    void add(Data&& data);

private:
    JsonFileHandler(std::fstream&& stream, Mode mode);

    Mode mode;
    std::map<int, std::shared_ptr<Data>> cache;
    std::fstream stream;
    std::string author;
};
}

#endif /* SRC_PERSISTENCE_JSONFILEHANDLER */
