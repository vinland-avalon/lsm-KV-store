cmake_minimum_required(VERSION 3.0.0)

#------------------------Main Function---------------------------

project(Lsm-Kv-Store VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(include)

include_directories(./third-party/spdlog/include)
link_directories(./third-party/spdlog/build)

set(CAMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(Lsm-Kv-Store main.cpp db/utils_for_file_operation.cpp db/utils_for_time_operation.cpp)

target_link_libraries(Lsm-Kv-Store spdlog)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# -------------------------Unit Test-------------------------------

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(./third-party/spdlog/include)
link_directories(./third-party/spdlog//build)

add_executable(FooTest test.cpp db/utils_for_file_operation.cpp db/utils_for_time_operation.cpp)


target_link_libraries(FooTest
        ${GTEST_BOTH_LIBRARIES}
        spdlog
        pthread )

gtest_discover_tests(FooTest)

message(STATUS "GTEST_INCLUDE_DIRS = ${GTEST_INCLUDE_DIRS}.")
message(STATUS "GTEST_BOTH_LIBRARIES = ${GTEST_BOTH_LIBRARIES}.")
message(STATUS "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}.")

