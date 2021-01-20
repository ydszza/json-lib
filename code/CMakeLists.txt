cmake_minimum_required (VERSION 3.0)

project (json)

add_executable(value_test test.cpp json.cpp value.cpp)
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})