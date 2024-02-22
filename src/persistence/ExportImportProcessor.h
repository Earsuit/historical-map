#ifndef SRC_PERSISTENCE_JSONFILEHANDLER
#define SRC_PERSISTENCE_JSONFILEHANDLER

#include "src/persistence/Data.h"

#include "spdlog/spdlog.h"
#include "tl/expected.hpp"

#include <set>
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

struct DataCompare
{
    bool operator()(const auto& lhs, const auto& rhs) const
    {
        return lhs.year < rhs.year;
    }
};

class ExportImportProcessor {
public:
    static tl::expected<std::unique_ptr<ExportImportProcessor>, Error> create(const std::string& file, Mode mode);

    ~ExportImportProcessor();

    void setAuthor(const std::string& author);
    const Data& front();
    void pop();
    bool empty();
    void insert(Data data);
    void insert(Data&& data);

private:
    ExportImportProcessor(std::fstream&& stream, Mode mode);

    Mode mode;
    std::fstream stream;
    std::set<Data, DataCompare> infos;
    std::string author;
    std::shared_ptr<spdlog::logger> logger;
};
}

#endif /* SRC_PERSISTENCE_JSONFILEHANDLER */
