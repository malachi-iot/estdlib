cmake_minimum_required(VERSION 3.5)

set(ESTDLIB_DIR ../../../..)

set(EXTRA_COMPONENT_DIRS 
    ${ESTDLIB_DIR}/tools/esp-idf/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${ESTDLIB_DIR}/tools/esp-idf/version_finder.cmake)

project(chrono)