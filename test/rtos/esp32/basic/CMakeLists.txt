# This is the project CMakeLists.txt file for the test subproject 
cmake_minimum_required(VERSION 3.5)

# as per https://esp32.com/viewtopic.php?t=10316
# To pass in the ESP32 CPPFlag
# Does not work, halts CMake process
#target_compile_definitions(${COMPONENT_TARGET} PUBLIC "-DESP32")

set(ESTDLIB_DIR ../../../..)
#get_filename_component(__ESTDLIB_DIR ${ESTDLIB_DIR} ABSOLUTE)
#set(ENV{ESTDLIB_DIR} ${__ESTDLIB_DIR})

set(EXTRA_COMPONENT_DIRS 
    ${ESTDLIB_DIR}/tools/esp-idf/component)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${ESTDLIB_DIR}/tools/esp-idf/version_finder.cmake)

project(basic)