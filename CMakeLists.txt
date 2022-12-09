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
