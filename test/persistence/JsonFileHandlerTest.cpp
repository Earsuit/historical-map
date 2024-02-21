#include "src/persistence/JsonFileHandler.h"
#include "src/persistence/Data.h"

#include "nlohmann/json.hpp"

#include <gtest/gtest.h>
#include <cstdio>

namespace {
constexpr auto FILE_NAME = "jsonFileHandlerTest.json";

class JsonFileHandlerTest : public ::testing::Test {
public:
    JsonFileHandlerTest()
    {
        std::remove(FILE_NAME);
    }

    ~JsonFileHandlerTest()
    {
        std::remove(FILE_NAME);
    }
};

TEST_F(JsonFileHandlerTest, ExportAndImport)
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

    if (auto handler = persistence::JsonFileHandler::create(FILE_NAME, persistence::Mode::Write); handler) {
        handler.value()->setAuthor("Test");
        for (const auto& info : infos) {
            handler.value()->add(info);
        }
    } else {
        EXPECT_TRUE(false); 
    }

    std::vector<persistence::Data> readback;

    if (auto handler = persistence::JsonFileHandler::create(FILE_NAME, persistence::Mode::Read); handler) {
        for (auto info = handler.value()->next(); info; info = handler.value()->next()) {
            readback.emplace_back(*info);
        }
    } else {
        EXPECT_TRUE(false); 
    }

    EXPECT_EQ(readback, infos);
}

TEST_F(JsonFileHandlerTest, NextOutputYearAssendingOrder)
{
    const std::vector<persistence::Data> infos{
        {100}, {10}, {-200}, {-300}, {1}, {2}
    };

    const std::vector<int> expected{
        -300, -200, 1, 2, 10, 100
    };

    if (auto handler = persistence::JsonFileHandler::create(FILE_NAME, persistence::Mode::Write); handler) {
        for (const auto& info : infos) {
            handler.value()->add(info);
        }
    } else {
        EXPECT_TRUE(false); 
    }

    if (auto handler = persistence::JsonFileHandler::create(FILE_NAME, persistence::Mode::Read); handler) {
        int i = 0;
        for (auto info = handler.value()->next(); info; info = handler.value()->next()) {
            EXPECT_EQ(expected[i++], info->year);
        }

        EXPECT_EQ(i, expected.size());
    } else {
        EXPECT_TRUE(false); 
    }
}

TEST_F(JsonFileHandlerTest, WriteFileExists)
{
    persistence::JsonFileHandler::create(FILE_NAME, persistence::Mode::Write);
    
    const auto handler = persistence::JsonFileHandler::create(FILE_NAME, persistence::Mode::Write);

    EXPECT_FALSE(handler);

    EXPECT_EQ(handler.error(), persistence::Error::FILE_EXISTS);
}

TEST_F(JsonFileHandlerTest, OverWrite)
{
    persistence::JsonFileHandler::create(FILE_NAME, persistence::Mode::Write);
    
    const auto ret = persistence::JsonFileHandler::create(FILE_NAME, persistence::Mode::Write);

    EXPECT_FALSE(ret);

    EXPECT_EQ(ret.error(), persistence::Error::FILE_EXISTS);

    const auto handler = persistence::JsonFileHandler::create(FILE_NAME, persistence::Mode::OverWrite);

    EXPECT_TRUE(handler);
}

TEST_F(JsonFileHandlerTest, ReadFileNotExists)
{
    const auto ret = persistence::JsonFileHandler::create("FILE_NAME", persistence::Mode::Read);

    EXPECT_FALSE(ret);

    EXPECT_EQ(ret.error(), persistence::Error::FILE_NOT_EXISTS);

}

}