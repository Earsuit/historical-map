#include "src/persistence/exporterImporter/ExporterImporterFactory.h"
#include "src/persistence/exporterImporter/JsonExporterImporter.h"
#include "src/persistence/Data.h"

#include "nlohmann/json.hpp"

#include <gtest/gtest.h>
#include <cstdio>
#include <string>
#include <sstream>

namespace {
constexpr auto FILE_NAME = "jsonFileHandlerTest.json";
constexpr auto FORMAT = "JSON";

class JsonExporterImporterTest : public ::testing::Test {
public:
    JsonExporterImporterTest()
    {
        std::remove(FILE_NAME);
    }

    ~JsonExporterImporterTest()
    {
        std::remove(FILE_NAME);
    }
};

TEST_F(JsonExporterImporterTest, ExportAndImport)
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
        importer.value()->loadFromFile(FILE_NAME);
        while (!importer.value()->empty()) {
            readback.emplace_back(importer.value()->front());
            importer.value()->pop();
        }
    } else {
        EXPECT_TRUE(false);
    }

    EXPECT_EQ(readback, infos);
}

TEST_F(JsonExporterImporterTest, NextOutputYearAscendingOrder)
{
    const std::vector<persistence::Data> infos{
        {100}, {10}, {-200}, {-300}, {1}, {2}
    };

    const std::vector<int> expected{
        -300, -200, 1, 2, 10, 100
    };

    if (auto exporter = persistence::ExporterImporterFactory::getInstance().createExporter(FORMAT); exporter) {
        for (const auto& info : infos) {
            exporter.value()->insert(info);
        }

        EXPECT_TRUE(exporter.value()->writeToFile(FILE_NAME, false));
    } else {
        EXPECT_TRUE(false); 
    }

    if (auto importer = persistence::ExporterImporterFactory::getInstance().createImporter(FORMAT); importer) {
        importer.value()->loadFromFile(FILE_NAME);
        int i = 0;
        while (!importer.value()->empty()) {
            EXPECT_EQ(expected[i++], importer.value()->front().year);
            importer.value()->pop();
        }

        EXPECT_EQ(i, expected.size());
    } else {
        EXPECT_TRUE(false); 
    }
}

TEST_F(JsonExporterImporterTest, WriteFileExists)
{
    if (auto exporter = persistence::ExporterImporterFactory::getInstance().createExporter(FORMAT); exporter) {
        EXPECT_TRUE(exporter.value()->writeToFile(FILE_NAME, false));

        auto ret = exporter.value()->writeToFile(FILE_NAME, false);
        EXPECT_FALSE(ret);

        EXPECT_EQ(ret.error().code, persistence::ErrorCode::FILE_EXISTS);
    } else {
        EXPECT_TRUE(false); 
    }
}

TEST_F(JsonExporterImporterTest, OverWrite)
{
    if (auto exporter = persistence::ExporterImporterFactory::getInstance().createExporter(FORMAT); exporter) {
        EXPECT_TRUE(exporter.value()->writeToFile(FILE_NAME, false));

        auto ret = exporter.value()->writeToFile(FILE_NAME, false);
        EXPECT_FALSE(ret);

        EXPECT_EQ(ret.error().code, persistence::ErrorCode::FILE_EXISTS);

        EXPECT_TRUE(exporter.value()->writeToFile(FILE_NAME, true));
    } else {
        EXPECT_TRUE(false); 
    }
}

TEST_F(JsonExporterImporterTest, ReadFileNotExists)
{
    if (auto importer = persistence::ExporterImporterFactory::getInstance().createImporter(FORMAT); importer) {
        importer.value()->loadFromFile(FILE_NAME);
    } else {
        EXPECT_TRUE(false); 
    }

    auto importer = persistence::ExporterImporterFactory::getInstance().createImporter(FORMAT);

    auto ret = importer.value()->loadFromFile(FILE_NAME);

    EXPECT_EQ(ret.error().code, persistence::ErrorCode::FILE_NOT_EXISTS);
}

TEST_F(JsonExporterImporterTest, IncorrectJsonFormat)
{
    if (auto importer = persistence::ExporterImporterFactory::getInstance().createImporter(FORMAT); importer) {
        auto ret = importer.value()->loadFromFile("incorrectJson.json");
        EXPECT_EQ(ret.error().code, persistence::ErrorCode::PARSE_FILE_ERROR);
        EXPECT_EQ(ret.error().msg, "[json.exception.out_of_range.403] key 'cities' not found");

        EXPECT_TRUE(importer.value()->empty());
    } else {
        EXPECT_TRUE(false); 
    }
}
}