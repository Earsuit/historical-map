add_executable(DatabaseTest DatabaseTest.cpp)
target_link_libraries(DatabaseTest PRIVATE libpersistence GTest::gtest_main)
gtest_discover_tests(DatabaseTest)

configure_file(incorrectJson.json ${CMAKE_CURRENT_BINARY_DIR}/incorrectJson.json COPYONLY)

add_executable(JsonExporterImporterTest JsonExporterImporterTest.cpp)
# need to export the symbol so that the auto registration can work
if (APPLE)
target_link_libraries(JsonExporterImporterTest PRIVATE -Wl,-force_load libpersistence liblogger GTest::gtest_main)
else()
target_link_libraries(JsonExporterImporterTest PRIVATE -Wl,--whole-archive libpersistence -Wl,--no-whole-archive liblogger GTest::gtest_main)
endif()
gtest_discover_tests(JsonExporterImporterTest)

add_executable(BsonExporterImporterTest BsonExporterImporterTest.cpp)
# need to export the symbol so that the auto registration can work
if (APPLE)
target_link_libraries(BsonExporterImporterTest PRIVATE -Wl,-force_load libpersistence liblogger GTest::gtest_main)
else()
target_link_libraries(BsonExporterImporterTest PRIVATE -Wl,--whole-archive libpersistence -Wl,--no-whole-archive liblogger GTest::gtest_main)
endif()
gtest_discover_tests(BsonExporterImporterTest)