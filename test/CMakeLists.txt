# GoogleTest: `gtest_discover_tests` discovers tests by asking the _compiled test executable_
# to enumerate its tests. `PRE_TEST` delays test discovery until just prior to test execution;
# this avoids calling the executables during the build phase.
set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)

add_subdirectory(persistence)
add_subdirectory(util)