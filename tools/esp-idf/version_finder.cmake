cmake_minimum_required(VERSION 3.10)

# From https://stackoverflow.com/questions/38823537/execute-git-describe-in-custom-target

# NOTE: Only use this if you really need < 4.0 detection.  For all other (most) cases,
# you will want https://github.com/espressif/esp-idf/blob/v4.4.2/components/esp_common/include/esp_idf_version.h 

SET(IDF_PATH $ENV{IDF_PATH})

#message(STATUS "cmake IDF_PATH = ${IDF_PATH}")

find_package(Git)

# Store version into variable
execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
    OUTPUT_VARIABLE IDF_VER
    WORKING_DIRECTORY ${IDF_PATH}
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )


#SET(IDF_VER v3.0.1-dev)
#SET(IDF_VER v3.0.1)

message(DEBUG "cmake IDF_VER = ${IDF_VER}")

#string(REGEX MATCHALL "^v([0-9]).([0-9])([.][0-9]|)(-[A-Za-z]+|)" TEST_OUT ${IDF_VER})
string(REGEX MATCHALL "^v([0-9]).([0-9])([.])?([0-9])?(-)?(.+)?" TEST_OUT ${IDF_VER})

set(IDF_VER_MAJOR ${CMAKE_MATCH_1})
set(IDF_VER_MINOR ${CMAKE_MATCH_2})
set(IDF_VER_PATCH ${CMAKE_MATCH_4})
set(IDF_VER_TAG ${CMAKE_MATCH_6})

if ("${IDF_VER_PATCH}" STREQUAL "")
    set(IDF_VER_PATCH 0)
endif()

if (NOT CMAKE_SCRIPT_MODE_FILE)
    message(STATUS "estd: esp-idf v${IDF_VER_MAJOR}.${IDF_VER_MINOR}.${IDF_VER_PATCH} (${IDF_VER_TAG})")
    # Not add_definitions or add_compile_definitions, as per
    # https://github.com/espressif/esp-idf/issues/4607
    add_compile_options(
        -DESTD_IDF_VER_MAJOR=${IDF_VER_MAJOR}
        -DESTD_IDF_VER_MINOR=${IDF_VER_MINOR}
        -DESTD_IDF_VER_PATCH=${IDF_VER_PATCH}
        )
else()
    message(DEBUG "cmake TEST_OUT=${TEST_OUT} / ${CMAKE_MATCH_1} + ${CMAKE_MATCH_2} + ${CMAKE_MATCH_3} + ${CMAKE_MATCH_4} + ${CMAKE_MATCH_5} + ${CMAKE_MATCH_6}")
endif()
