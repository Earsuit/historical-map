add_executable(TypeTraitsTest TypeTraitsTest.cpp)
target_link_libraries(TypeTraitsTest PRIVATE GTest::gtest_main)
gtest_add_tests(TARGET TypeTraitsTest)

add_executable(SignalTest SignalTest.cpp)
target_link_libraries(SignalTest PRIVATE GTest::gtest_main)
gtest_add_tests(TARGET SignalTest)