#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_JSON_EXPORTER_IMPORTER_CPP
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_JSON_EXPORTER_IMPORTER_CPP
#include "src/persistence/exporterImporter/JsonExporterImporter.h"

#include <optional>

namespace persistence {
constexpr auto PRETTIFY_JSON = 4;
constexpr auto JSON_FIRST_LEVEL_NAME = "historical_info";

void to_json(nlohmann::json& j, const Coordinate& c) {
    j = nlohmann::json{{"latitude", c.latitude}, {"longitude", c.longitude}};
}

void from_json(const nlohmann::json& j, Coordinate& c) {
    j.at("latitude").get_to(c.latitude);
    j.at("longitude").get_to(c.longitude);
}

void to_json(nlohmann::json& j, const Country& c) {
    j = nlohmann::json{{"name", c.name}, {"contour", c.borderContour}};
}

void from_json(const nlohmann::json& j, Country& c) {
    j.at("name").get_to(c.name);
    j.at("contour").get_to(c.borderContour);
}

void to_json(nlohmann::json& j, const City& c) {
    j = nlohmann::json{{"name", c.name}, {"coordinate", c.coordinate}};
}

void from_json(const nlohmann::json& j, City& c) {
    j.at("name").get_to(c.name);
    j.at("coordinate").get_to(c.coordinate);
}

void to_json(nlohmann::json& j, const Note& n) {    j = nlohmann::json{{"text", n.text}};
}

void from_json(const nlohmann::json& j, Note& n) {
    j.at("text").get_to(n.text);
}

void to_json(nlohmann::json& j, const Data& d) {
    j = nlohmann::json{{"year", d.year}, {"countries", d.countries}, {"cities", d.cities}};

    if (d.note) {
        j["note"] = d.note.value();
    }
}

void from_json(const nlohmann::json& j, Data& d) {
    j.at("year").get_to(d.year);
    j.at("countries").get_to(d.countries);
    j.at("cities").get_to(d.cities);
    if (j.count("note") != 0) {
        d.note = j.at("note").get<Note>();
    }
}

JsonExporter::JsonExporter()
{
    json[JSON_FIRST_LEVEL_NAME] = {};
}

void JsonExporter::insert(const Data& info)
{
    json[JSON_FIRST_LEVEL_NAME].emplace_back(info);
}

void JsonExporter::insert(Data&& info)
{
    json[JSON_FIRST_LEVEL_NAME].emplace_back(std::move(info));
}

util::Expected<void> JsonExporter::writeToFile(const std::string& file, bool overwrite)
{
    if (auto&& ret = openFile(file, overwrite); ret) {
        toStream(std::move(ret).value(), json);
    } else {
        return util::Unexpected{ret.error()};;
    }

    return util::SUCCESS;
}

void JsonExporter::toStream(std::fstream stream, const nlohmann::json& json)
{
    stream << std::setw(PRETTIFY_JSON) << json << std::endl;
}

util::Expected<std::fstream> JsonExporter::openFile(const std::string& file, bool overwrite)
{
    if (!overwrite && std::filesystem::exists(file)) {
        return util::Unexpected(util::Error{util::ErrorCode::FILE_EXISTS});
    }

    return std::fstream{file, std::ios::out | std::ios::trunc};
}

util::Generator<util::Expected<Data>> JsonImporter::loadFromFile(const std::string file)
{
    if (auto&& ret = openFile(file); ret) {
        std::optional<util::Error> error;
        try {
            const auto json = parse(std::move(ret).value());

            for (const auto& info : json[JSON_FIRST_LEVEL_NAME]) {
                co_yield info.template get<Data>();
            }
        }
        catch (const nlohmann::json::exception& e) {
            error = {util::ErrorCode::PARSE_FILE_ERROR, e.what()};
        }

        if (error) {
            co_yield util::Unexpected{*error};
        }
    } else {
        co_yield util::Unexpected{ret.error()};
    }
}

util::Expected<std::fstream> JsonImporter::openFile(const std::string& file)
{
    if (!std::filesystem::exists(file)) {
        return util::Unexpected(util::Error{util::ErrorCode::FILE_NOT_EXISTS});
    }

    return std::fstream{file, std::ios::in};
}

nlohmann::json JsonImporter::parse(std::fstream stream)
{
    return nlohmann::json::parse(stream);
}
}

#endif
