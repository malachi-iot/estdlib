# This is the project CMakeLists.txt file for the test subproject 
cmake_minimum_required(VERSION 3.5)

set(ESTDLIB_DIR ../../../..)

include(${ESTDLIB_DIR}/tools/esp-idf/project.cmake)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(stack-diagnostic)