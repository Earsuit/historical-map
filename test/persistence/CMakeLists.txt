add_executable(DatabaseTest DatabaseTest.cpp)
target_link_libraries(DatabaseTest PRIVATE GTest::gtest_main sqlpp11::sqlite3)
gtest_discover_tests(DatabaseTest)