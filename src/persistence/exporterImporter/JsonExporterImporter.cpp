#ifndef SRC_PERSISTENCE_EXPORTER_IMPORTER_JSON_EXPORTER_IMPORTER_CPP
#define SRC_PERSISTENCE_EXPORTER_IMPORTER_JSON_EXPORTER_IMPORTER_CPP
#include "src/persistence/exporterImporter/JsonExporterImporter.h"
#include "src/persistence/exporterImporter/ExporterImporterRegistrar.h"

#include "nlohmann/json.hpp"

namespace persistence {
constexpr auto PRETTIFY_JSON = 4;

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
    j = nlohmann::json{{"year", d.year}, {"countries", d.countries}, {"cities", d.cities}, {"note", d.note}};
}

void from_json(const nlohmann::json& j, Data& d) {
    j.at("year").get_to(d.year);
    j.at("countries").get_to(d.countries);
    j.at("cities").get_to(d.cities);
    j.at("note").get_to(d.note);
}

void JsonExporter::insert(const Data& info)
{
    infos.emplace(std::move(info));
}

void JsonExporter::insert(Data&& info)
{
    infos.emplace(std::move(info));
}

tl::expected<void, Error> JsonExporter::writeToFile(const std::string& file, bool overwrite)
{
    if (const auto ret = openFile(file, overwrite); ret) {
        nlohmann::json json;
        const auto date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        json["historical_info"] = {};

        for (const auto& info : infos) {
            json["historical_info"].emplace_back(info);
        }

        stream << std::setw(PRETTIFY_JSON) << json << std::endl;
    } else {
        return ret;
    }

    return SUCCESS;
}

tl::expected<void, Error> JsonExporter::openFile(const std::string& file, bool overwrite)
{
    if (!overwrite && std::filesystem::exists(file)) {
        return tl::unexpected(Error{ErrorCode::FILE_EXISTS});
    }

    stream = std::fstream{file, std::ios::out | std::ios::trunc};

    return SUCCESS;
}

const Data& JsonImporter::front() const noexcept
{
    return *infos.begin();
}

void JsonImporter::pop()
{
    infos.erase(infos.begin());
}

bool JsonImporter::empty() const noexcept
{
    return infos.empty();
}

tl::expected<void, Error> JsonImporter::loadFromFile(const std::string& file)
{
    if (const auto ret = openFile(file); ret) {
        try {
            const auto json = nlohmann::json::parse(stream);

            for (const auto& info : json["historical_info"]) {
                infos.insert(info.template get<Data>());
            }
        }
        catch (const nlohmann::json::exception& e) {
            return tl::unexpected(Error{ErrorCode::PARSE_FILE_ERROR, e.what()});
        }
    } else {
        return ret;
    }

    return SUCCESS;
}

tl::expected<void, Error> JsonImporter::openFile(const std::string& file)
{
    if (!std::filesystem::exists(file)) {
        return tl::unexpected(Error{ErrorCode::FILE_NOT_EXISTS});
    }

    stream = std::fstream{file, std::ios::in};

    return SUCCESS;
}

static ExporterImporterRegistrar<JsonImporter, JsonExporter> jsonExporterImporterRegistrar{"JSON"};
}


#endif
