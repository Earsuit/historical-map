#include "src/persistence/ExportImportProcessor.h"
#include "src/persistence/Data.h"
#include "src/logger/Util.h"

#include "spdlog/sinks/ostream_sink.h"
#include "nlohmann/json.hpp"

#include <gtest/gtest.h>
#include <cstdio>
#include <sstream>

namespace {
constexpr auto FILE_NAME = "jsonFileHandlerTest.json";

class ExportImportProcessorTest : public ::testing::Test {
public:
    ExportImportProcessorTest():
        logger{std::make_shared<spdlog::logger>(logger::LOGGER_NAME, std::make_shared<spdlog::sinks::ostream_sink_mt>(stream))}
    {
        std::remove(FILE_NAME);

        spdlog::initialize_logger(logger);
        logger->set_pattern("%v");
    }

    ~ExportImportProcessorTest()
    {
        // std::remove(FILE_NAME);
    }

    std::ostringstream  stream;
    std::shared_ptr<spdlog::logger> logger;
};

TEST_F(ExportImportProcessorTest, ExportAndImport)
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

    if (auto handler = persistence::ExportImportProcessor::template create<persistence::Mode::Write>(FILE_NAME); handler) {
        handler.value()->setAuthor("Test");
        for (const auto& info : infos) {
            handler.value()->insert(info);
        }
    } else {
        EXPECT_TRUE(false);
    }

    EXPECT_EQ("", stream.str());
    std::vector<persistence::Data> readback;

    if (auto handler = persistence::ExportImportProcessor::template create<persistence::Mode::Read>(FILE_NAME); handler) {
        while (!handler.value()->empty()) {
            readback.emplace_back(handler.value()->front());
            handler.value()->pop();
        }
    } else {
        EXPECT_TRUE(false);
    }

    EXPECT_EQ("", stream.str());
    EXPECT_EQ(readback, infos);
}

TEST_F(ExportImportProcessorTest, NextOutputYearAssendingOrder)
{
    const std::vector<persistence::Data> infos{
        {100}, {10}, {-200}, {-300}, {1}, {2}
    };

    const std::vector<int> expected{
        -300, -200, 1, 2, 10, 100
    };

    if (auto handler = persistence::ExportImportProcessor::template create<persistence::Mode::Write>(FILE_NAME); handler) {
        for (const auto& info : infos) {
            handler.value()->insert(info);
        }
    } else {
        EXPECT_TRUE(false); 
    }

    if (auto handler = persistence::ExportImportProcessor::template create<persistence::Mode::Read>(FILE_NAME); handler) {
        int i = 0;
        while (!handler.value()->empty()) {
            EXPECT_EQ(expected[i++], handler.value()->front().year);
            handler.value()->pop();
        }

        EXPECT_EQ(i, expected.size());
    } else {
        EXPECT_TRUE(false); 
    }
}

TEST_F(ExportImportProcessorTest, WriteFileExists)
{
    persistence::ExportImportProcessor::template create<persistence::Mode::Write>(FILE_NAME);
    
    const auto handler = persistence::ExportImportProcessor::template create<persistence::Mode::Write>(FILE_NAME);

    EXPECT_FALSE(handler);

    EXPECT_EQ(handler.error(), persistence::Error::FILE_EXISTS);
}

TEST_F(ExportImportProcessorTest, OverWrite)
{
    persistence::ExportImportProcessor::template create<persistence::Mode::Write>(FILE_NAME);
    
    const auto ret = persistence::ExportImportProcessor::template create<persistence::Mode::Write>(FILE_NAME);

    EXPECT_FALSE(ret);

    EXPECT_EQ(ret.error(), persistence::Error::FILE_EXISTS);

    const auto handler = persistence::ExportImportProcessor::template create<persistence::Mode::OverWrite>(FILE_NAME);

    EXPECT_TRUE(handler);
}

TEST_F(ExportImportProcessorTest, ReadFileNotExists)
{
    const auto ret = persistence::ExportImportProcessor::template create<persistence::Mode::Read>("FILE_NAME");

    EXPECT_FALSE(ret);

    EXPECT_EQ(ret.error(), persistence::Error::FILE_NOT_EXISTS);
}

TEST_F(ExportImportProcessorTest, IncorrectJsonFormat)
{
    if (auto handler = persistence::ExportImportProcessor::template create<persistence::Mode::Read>("incorrectJson.json"); handler) {
        EXPECT_EQ("Failed to import file: [json.exception.out_of_range.403] key 'cities' not found\n", stream.str());

        EXPECT_TRUE(handler.value()->empty());
    } else {
        EXPECT_TRUE(false); 
    }
}
}