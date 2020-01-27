# This is the project CMakeLists.txt file for the test subproject 
cmake_minimum_required(VERSION 3.5)

# as per https://esp32.com/viewtopic.php?t=10316
# To pass in the ESP32 CPPFlag
# Does not work, halts CMake process
#target_compile_definitions(${COMPONENT_TARGET} PUBLIC "-DESP32")

# Set the components to include the tests for.
# This can be overriden from CMake cache:
# - when invoking CMake directly: cmake -D TEST_COMPONENTS="xxxxx" ..
# - when using idf.py: idf.py build -T xxxxx
#
set(TEST_COMPONENTS "testable" CACHE STRING "List of components to test")

set(ESTDLIB_DIR ../../../..)

include(${ESTDLIB_DIR}/tools/esp-idf/project.cmake NO_POLICY_SCOPE)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

message("MAIN: ${ESTDLIB_DIR}")

project(unity)