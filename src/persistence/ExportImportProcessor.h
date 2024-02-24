#ifndef SRC_PERSISTENCE_EXPORT_IMPORT_PROCESSOR_H
#define SRC_PERSISTENCE_EXPORT_IMPORT_PROCESSOR_H

#include "src/persistence/Data.h"
#include "src/logger/Util.h"

#include "spdlog/spdlog.h"
#include "tl/expected.hpp"
#include "nlohmann/json.hpp"

#include <set>
#include <memory>
#include <fstream>
#include <type_traits>
#include <filesystem>
#include <chrono>

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

template<Mode mode>
class ExportImportProcessorImpl;

class ExportImportProcessor {
public:
    template<Mode mode>
    static tl::expected<std::unique_ptr<ExportImportProcessorImpl<mode>>, Error> create(const std::string& file)
    {
        if constexpr (mode == Mode::Write) {
            if (std::filesystem::exists(file)) {
                return tl::unexpected(Error::FILE_EXISTS);
            }

            return std::unique_ptr<ExportImportProcessorImpl<mode>>(new ExportImportProcessorImpl<mode>{std::fstream{file, std::ios::out | std::ios::trunc}});
        }

        if constexpr (mode == Mode::OverWrite) {
            return std::unique_ptr<ExportImportProcessorImpl<mode>>(new ExportImportProcessorImpl<mode>{std::fstream{file, std::ios::out | std::ios::trunc}});
        }

        if constexpr (mode == Mode::Read) {
            if (!std::filesystem::exists(file)) {
                return tl::unexpected(Error::FILE_NOT_EXISTS);
            }

            return std::unique_ptr<ExportImportProcessorImpl<mode>>(new ExportImportProcessorImpl<mode>{std::fstream{file, std::ios::in}});
        }

        return tl::unexpected(Error::INVALID_MODE);
    }

    void setAuthor(const std::string& author)
    {
        this->author = author;
    }

protected:
    std::fstream stream;
    std::set<Data, DataCompare> infos;
    std::string author;
    std::shared_ptr<spdlog::logger> logger;

    ExportImportProcessor(std::fstream&& s):
        stream{std::move(s)},
        logger{spdlog::get(logger::LOGGER_NAME)}
    {
    }
};

template<Mode mode>
class ExportImportProcessorImpl: public ExportImportProcessor {
public:
    ExportImportProcessorImpl(std::fstream&& s):
        ExportImportProcessor{std::move(s)}
    {
    }

    ~ExportImportProcessorImpl()
    {
        nlohmann::json json;
        const auto date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        json["author"] = author;
        json["date"] = std::ctime(&date);
        json["historical_info"] = {};

        for (const auto& info : infos) {
            json["historical_info"].emplace_back(info);
        }

        stream << std::setw(PRETTIFY_JSON) << json << std::endl;
    }

    void insert(Data info)
    {
        infos.emplace(std::move(info));
    }

    void insert(Data&& info)
    {
        infos.emplace(std::move(info));
    }
};

template<>
class ExportImportProcessorImpl<Mode::Read>: public ExportImportProcessor {
public:
     ExportImportProcessorImpl(std::fstream&& s):
        ExportImportProcessor{std::move(s)}
    {
        try {
            const auto json = nlohmann::json::parse(stream);

            for (const auto& info : json["historical_info"]) {
                infos.insert(info.template get<Data>());
            }
        }
        catch (const nlohmann::json::exception& e) {
            logger->error("Failed to import file: {}", e.what());
        }
    }

    const Data& front()
    {
        return *infos.begin();
    }

    void pop()
    {
        infos.erase(infos.begin());
    }

    bool empty()
    {
        return infos.empty();
    }
};

}

#endif
