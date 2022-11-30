cmake_minimum_required(VERSION 3.22)

project(Tutorial)
add_executable(Tutorial tutorial.cpp)

add_subdirectory(./sstable/json)

target_link_libraries(Tutorial PRIVATE nlohmann_json::nlohmann_json)
