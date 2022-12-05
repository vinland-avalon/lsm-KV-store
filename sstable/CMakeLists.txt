cmake_minimum_required(VERSION 3.22)

project(Test_Ss_Table)
add_executable(Test_Ss_Table ss_table.cpp table_meta_info.cpp ./ss_table_test.cpp ../utils/utils_for_file_operation.cpp)

add_subdirectory(./json)

target_link_libraries(Test_Ss_Table PRIVATE nlohmann_json::nlohmann_json)
