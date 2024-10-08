#include "src/persistence/exporterImporter/BsonExporterImporter.h"

#include <filesystem>
#include <iterator>

namespace persistence {
void BsonExporter::toStream(std::fstream stream, const nlohmann::json& json)
{
    std::vector<std::uint8_t> binary = nlohmann::json::to_bson(json);

    stream.write(reinterpret_cast<const char*>(binary.data()), binary.size());

    stream.flush();
}

util::Expected<std::fstream> BsonExporter::openFile(const std::string& file, bool overwrite)
{
    if (!overwrite && std::filesystem::exists(file)) {
        return util::Unexpected(util::Error{util::ErrorCode::FILE_EXISTS});
    }

    return std::fstream{file, std::ios::out | std::ios::trunc | std::ios::binary};
}

util::Expected<std::fstream> BsonImporter::openFile(const std::string& file)
{
    if (!std::filesystem::exists(file)) {
        return util::Unexpected(util::Error{util::ErrorCode::FILE_NOT_EXISTS});
    }

    return std::fstream{file, std::ios::in | std::ios::binary};
}

nlohmann::json BsonImporter::parse(std::fstream stream)
{
    std::vector<uint8_t> binary{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};

    return nlohmann::json::from_bson(binary);
}
}