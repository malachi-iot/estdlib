# This is the project CMakeLists.txt file for the test subproject 
cmake_minimum_required(VERSION 3.5)

# as per https://esp32.com/viewtopic.php?t=10316
# To pass in the ESP32 CPPFlag
# Does not work, halts CMake process
#target_compile_definitions(${COMPONENT_TARGET} PUBLIC "-DESP32")

set(EXTRA_COMPONENT_DIRS "components-esp-idf-3.3")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(basic)