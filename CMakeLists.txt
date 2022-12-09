cmake_minimum_required(VERSION 3.0.0)
project(Lsm-Kv-Store VERSION 0.1.0)

include(CTest)
enable_testing()

# add_subdirectory(db)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(include)
# link_directories(db/build)
# aux_source_directory(db DB)
# set(DBsrc db/utils_for_file_operation.cpp db/utils_for_time_operation.cpp)

set(CAMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(Lsm-Kv-Store main.cpp db/utils_for_file_operation.cpp db/utils_for_time_operation.cpp) 




set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# add_executable(
#   test_0
#   test/test_0.cpp
# )

# include(GoogleTest)
# gtest_discover_tests(hello_test)

# target_link_libraries(Lsm-Kv-Store libdb.so)

# ---------------------------test--------------------------------

# 查找 GTest 库
find_package(GTest REQUIRED)
# GTest 的头文件
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(FooTest test.cpp db/utils_for_file_operation.cpp db/utils_for_time_operation.cpp)

# 链接测试库
target_link_libraries(FooTest
        ${GTEST_BOTH_LIBRARIES}
        pthread )

# 添加到测试
gtest_discover_tests(FooTest)


