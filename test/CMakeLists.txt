include(CTest)

# set test build target name
set(TEST_BINARY ${PROJECT_NAME}_test)

# add new test
add_executable(${TEST_BINARY} test_0.cpp)

# add googletest subdirectory
# add_subdirectory(googletest-master)

# this is so we can #include <gtest/gtest.h>
# include_directories(googletest-master/include)

add_test(test_1 test/test_0.cpp)

# link libraries to use in this executable
# link gtest_main to avoid your own main.cpp
target_link_libraries(${TEST_BINARY} PUBLIC libTest gtest gtest_main)
