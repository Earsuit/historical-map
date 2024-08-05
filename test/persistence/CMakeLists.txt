add_executable(DatabaseTest DatabaseTest.cpp)
target_link_libraries(DatabaseTest PRIVATE libpersistence GTest::gtest_main)
gtest_add_tests(TARGET DatabaseTest)

configure_file(incorrectJson.json ${CMAKE_CURRENT_BINARY_DIR}/incorrectJson.json COPYONLY)

add_executable(JsonExporterImporterTest JsonExporterImporterTest.cpp)
# need to export the symbol so that the auto registration can work
if (APPLE)
target_link_libraries(JsonExporterImporterTest PRIVATE -Wl,-force_load libpersistence liblogger GTest::gtest_main)
elseif(LINUX)
target_link_libraries(JsonExporterImporterTest PRIVATE -Wl,--whole-archive libpersistence -Wl,--no-whole-archive liblogger GTest::gtest_main)
elseif(MSVC)
target_link_libraries(JsonExporterImporterTest PRIVATE liblogger GTest::gtest_main libpersistence)
set_property(TARGET JsonExporterImporterTest APPEND_STRING PROPERTY LINK_FLAGS " /WHOLEARCHIVE:libpersistence")
endif()
gtest_add_tests(TARGET JsonExporterImporterTest)

add_executable(BsonExporterImporterTest BsonExporterImporterTest.cpp)
# need to export the symbol so that the auto registration can work
if (APPLE)
target_link_libraries(BsonExporterImporterTest PRIVATE -Wl,-force_load libpersistence liblogger GTest::gtest_main)
elseif(LINUX)
target_link_libraries(BsonExporterImporterTest PRIVATE -Wl,--whole-archive libpersistence -Wl,--no-whole-archive liblogger GTest::gtest_main)
elseif(MSVC)
target_link_libraries(BsonExporterImporterTest PRIVATE liblogger GTest::gtest_main libpersistence)
set_property(TARGET BsonExporterImporterTest APPEND_STRING PROPERTY LINK_FLAGS " /WHOLEARCHIVE:libpersistence")
endif()
gtest_add_tests(TARGET BsonExporterImporterTest)