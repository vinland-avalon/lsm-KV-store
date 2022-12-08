cmake_minimum_required(VERSION 3.0.0)
project(db VERSION 0.1.0)

include(CTest)
enable_testing()

aux_source_directory(. src)

add_library(db SHARED ${src})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
