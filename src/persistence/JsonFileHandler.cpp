#include "src/persistence/JsonFileHandler.h"

#include "nlohmann/json.hpp"

#include <filesystem>
#include <chrono>

namespace persistence {
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

void to_json(nlohmann::json& j, const Note& n) {
    j = nlohmann::json{{"note", n.text}};
}

void from_json(const nlohmann::json& j, Note& n) {
    j.at("note").get_to(n.text);
}

void to_json(nlohmann::json& j, const Data& d) {
    j = nlohmann::json{{"year", d.year}};
    j = nlohmann::json{{"countries", d.countries}};
    j = nlohmann::json{{"cities", d.cities}};
    j = nlohmann::json{{"note", d.note}};
}

void from_json(const nlohmann::json& j, Data& d) {
    j.at("year").get_to(d.year);
    j.at("countries").get_to(d.countries);
    j.at("cities").get_to(d.cities);
    j.at("note").get_to(d.note);
}

tl::expected<std::unique_ptr<JsonFileHandler>, Error> JsonFileHandler::create(const std::string& file, Mode mode)
{
    switch (mode) {
        case Mode::Write:
            if (std::filesystem::exists(file)) {
                return tl::unexpected(Error::FILE_EXISTS);
            }

            return std::unique_ptr<JsonFileHandler>(new JsonFileHandler{std::fstream{file, std::ios::out}, mode});
            break;
        case Mode::Read:
            if (!std::filesystem::exists(file)) {
                return tl::unexpected(Error::FILE_NOT_EXISTS);
            }

            return std::unique_ptr<JsonFileHandler>(new JsonFileHandler{std::fstream{file, std::ios::in}, mode});
            break;
        case Mode::OverWrite:
            return std::unique_ptr<JsonFileHandler>(new JsonFileHandler{std::fstream{file, std::ios::out | std::ios::trunc}, mode});
            break;
        default:
            return tl::unexpected(Error::INVALID_MODE);
    }
}

JsonFileHandler::JsonFileHandler(std::fstream&& stream, Mode mode):
    stream{std::move(stream)},
    mode{mode}
{
    if (mode == Mode::Read) {
        const auto json = nlohmann::json::parse(stream);

        for (const auto& info : json["historical_info"]) {
            auto data = std::make_shared<Data>(info.template get<Data>());
        }
    }
}

void JsonFileHandler::setAuthor(const std::string& author)
{
    this->author = author;
}

JsonFileHandler::~JsonFileHandler()
{
    if (mode != Mode::Read) {
        nlohmann::json json;
        const auto date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        json["author"] = author;
        json["date"] = std::ctime(&date);
        json["historical_info"] = {};

        for (const auto& [year, info] : cache) {
            json["historical_info"].emplace_back(*info);
        }

        stream << json << std::endl;
    }
}

std::shared_ptr<Data> JsonFileHandler::load(int year)
{
    if (cache.contains(year)) {
        return cache[year];
    } else {
        return nullptr;
    }
}

void JsonFileHandler::add(Data data)
{
    cache[data.year] = std::make_shared<Data>(data);
}

void JsonFileHandler::add(Data&& data)
{
    cache[data.year] = std::make_shared<Data>(std::move(data));
}
}