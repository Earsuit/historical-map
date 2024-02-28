#include "src/persistence/exporterImporter/BsonExporterImporter.h"

#include <filesystem>
#include <iterator>

namespace persistence {
tl::expected<void, Error> BsonExporter::writeToFile(const std::string& file, bool overwrite)
{
    if (const auto ret = openFile(file, overwrite); ret) {
        std::vector<std::uint8_t> binary = nlohmann::json::to_bson(toJson());

        stream.write(reinterpret_cast<const char*>(binary.data()), binary.size());
    } else {
        return ret;
    }

    return SUCCESS;
}

tl::expected<void, Error> BsonExporter::openFile(const std::string& file, bool overwrite)
{
    if (!overwrite && std::filesystem::exists(file)) {
        return tl::unexpected(Error{ErrorCode::FILE_EXISTS});
    }

    stream = std::fstream{file, std::ios::out | std::ios::trunc | std::ios::binary};

    return SUCCESS;
}

tl::expected<void, Error> BsonImporter::openFile(const std::string& file)
{
    if (!std::filesystem::exists(file)) {
        return tl::unexpected(Error{ErrorCode::FILE_NOT_EXISTS});
    }

    stream = std::fstream{file, std::ios::in | std::ios::binary};

    return SUCCESS;
}

tl::expected<void, Error> BsonImporter::loadFromFile(const std::string& file)
{
    if (const auto ret = openFile(file); ret) {
        try {
            std::vector<uint8_t> binary{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};

            fromJson(nlohmann::json::from_bson(binary));
        }
        catch (const nlohmann::json::exception& e) {
            return tl::unexpected(Error{ErrorCode::PARSE_FILE_ERROR, e.what()});
        }
    } else {
        return ret;
    }

    return SUCCESS;
}
}