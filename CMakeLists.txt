cmake_minimum_required (VERSION 3.0)

project (ydsjson)

aux_source_directory(src SRC_LIST1)
aux_source_directory(test SRC_LIST2)

add_executable(ydsjson_test ${SRC_LIST1} ${SRC_LIST2})
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})