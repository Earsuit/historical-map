#include "src/persistence/exporterImporter/ExporterImporterFactory.h"
#include "src/persistence/Data.h"

#include "nlohmann/json.hpp"

#include <gtest/gtest.h>
#include <cstdio>
#include <string>

namespace {
constexpr auto FILE_NAME = "bsonExporterImporterTest.bson";
constexpr auto FORMAT = "bson";

class BsonExporterImporterTest : public ::testing::Test {
public:
    BsonExporterImporterTest()
    {
        std::remove(FILE_NAME);
    }

    ~BsonExporterImporterTest()
    {
        std::remove(FILE_NAME);
    }
};

TEST_F(BsonExporterImporterTest, ExportAndImport)
{
    const std::vector<persistence::Data> infos{
        persistence::Data{
            -200,
            std::list<persistence::Country>{
                persistence::Country{
                    "A",
                    std::list<persistence::Coordinate>{
                        persistence::Coordinate{
                            1.1f, 
                            2.1f
                        },
                        persistence::Coordinate{
                            1.0f, 
                            2.0f
                        },
                    }
                },
                persistence::Country{
                    "B",
                    std::list<persistence::Coordinate>{
                        persistence::Coordinate{
                            2.1f, 
                            3.1f
                        },
                        persistence::Coordinate{
                            4.0f, 
                            5.0f
                        },
                    }
                }
            },
            std::list<persistence::City>{
                persistence::City{
                    "C",
                    persistence::Coordinate{
                        10.0f,
                        11.0f
                    }
                },
                persistence::City{
                    "D",
                    persistence::Coordinate{
                        10.1f,
                        11.1f
                    }
                },
            },
            persistence::Note{"Note1"}
        },
        persistence::Data{
            200,
            std::list<persistence::Country>{
                persistence::Country{
                    "A",
                    std::list<persistence::Coordinate>{
                        persistence::Coordinate{
                            11.1f, 
                            21.1f
                        },
                        persistence::Coordinate{
                            11.0f, 
                            21.0f
                        },
                    }
                },
                persistence::Country{
                    "B",
                    std::list<persistence::Coordinate>{
                        persistence::Coordinate{
                            21.1f, 
                            31.1f
                        },
                        persistence::Coordinate{
                            41.0f, 
                            51.0f
                        },
                    }
                }
            },
            std::list<persistence::City>{
                persistence::City{
                    "C",
                    persistence::Coordinate{
                        101.0f,
                        111.0f
                    }
                },
                persistence::City{
                    "D",
                    persistence::Coordinate{
                        101.1f,
                        111.1f
                    }
                },
            },
            persistence::Note{"Note2"}
        },
    };

    if (auto exporter = persistence::ExporterImporterFactory::getInstance().createExporter(FORMAT); exporter) {
        for (const auto& info : infos) {
            exporter.value()->insert(info);
        }

        EXPECT_TRUE(exporter.value()->writeToFile(FILE_NAME, false));
    } else {
        EXPECT_TRUE(false);
    }

    std::vector<persistence::Data> readback;

    if (auto importer = persistence::ExporterImporterFactory::getInstance().createImporter(FORMAT); importer) {
        auto loader = importer.value()->loadFromFile(FILE_NAME);

        while (loader.next()) {
            if (const auto& ret = loader.getValue(); ret) {
                readback.emplace_back(ret.value());
            } else {
                EXPECT_TRUE(false);
            }
        }
    } else {
        EXPECT_TRUE(false);
    }

    EXPECT_EQ(readback, infos);
}

TEST_F(BsonExporterImporterTest, WriteFileExists)
{
    if (auto exporter = persistence::ExporterImporterFactory::getInstance().createExporter(FORMAT); exporter) {
        EXPECT_TRUE(exporter.value()->writeToFile(FILE_NAME, false));

        auto ret = exporter.value()->writeToFile(FILE_NAME, false);
        EXPECT_FALSE(ret);

        EXPECT_EQ(ret.error().code, util::ErrorCode::FILE_EXISTS);
    } else {
        EXPECT_TRUE(false); 
    }
}

TEST_F(BsonExporterImporterTest, OverWrite)
{
    if (auto exporter = persistence::ExporterImporterFactory::getInstance().createExporter(FORMAT); exporter) {
        EXPECT_TRUE(exporter.value()->writeToFile(FILE_NAME, false));

        auto ret = exporter.value()->writeToFile(FILE_NAME, false);
        EXPECT_FALSE(ret);

        EXPECT_EQ(ret.error().code, util::ErrorCode::FILE_EXISTS);

        EXPECT_TRUE(exporter.value()->writeToFile(FILE_NAME, true));
    } else {
        EXPECT_TRUE(false); 
    }
}

TEST_F(BsonExporterImporterTest, ReadFileNotExists)
{
    auto importer = persistence::ExporterImporterFactory::getInstance().createImporter(FORMAT);

    auto loader = importer.value()->loadFromFile(FILE_NAME);
    
    loader.next();

    EXPECT_EQ(loader.getValue().error().code, util::ErrorCode::FILE_NOT_EXISTS);
}

TEST_F(BsonExporterImporterTest, IncorrectJsonFormat)
{
    if (auto importer = persistence::ExporterImporterFactory::getInstance().createImporter(FORMAT); importer) {
        auto loader = importer.value()->loadFromFile("incorrectJson.json");
        std::optional<util::Error> error;

        while (loader.next()) {
            if (const auto& ret = loader.getValue(); ret) {
                continue;
            } else {
                error = ret.error();
            }
        }
        
        if (error) {
            EXPECT_EQ(error.value().code, util::ErrorCode::PARSE_FILE_ERROR);
            EXPECT_EQ(error.value().msg, "[json.exception.parse_error.110] parse error at byte 230: syntax error while parsing BSON cstring: unexpected end of input");
        } else {
            EXPECT_TRUE(false); 
        }
    } else {
        EXPECT_TRUE(false); 
    }
}
}