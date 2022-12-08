cmake_minimum_required(VERSION 3.0.0)
project(Lsm-Kv-Store VERSION 0.1.0)

include(CTest)
enable_testing()

add_subdirectory(src)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(Lsm-Kv-Store main.cpp)
target_link_libraries(Lsm-Kv-Store db)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# add_executable(
#   test_0
#   test/test_0.cpp
# )

# include(GoogleTest)
# gtest_discover_tests(hello_test)
