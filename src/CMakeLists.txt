# All references see /docs/references.md

cmake_minimum_required(VERSION 3.13)

project(estd VERSION 0.4.0 LANGUAGES CXX)

set(TOOLS_DIR ../tools/cmake)

include(${TOOLS_DIR}/platform.cmake)
include(${TOOLS_DIR}/includes.cmake)

set(SOURCE_FILES
    estd/internal/string_convert.cpp
    estd/internal/utility.cpp
    estd/port/posix/streambuf.cpp
    )


# Guidance from [2.1], otherwise we get INTERFACE_INCLUDE_DIRECTORIES property error
set(ESTD_INCLUDE_DIRECTORY
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
        $<INSTALL_INTERFACE:.>)

message(DEBUG "estd: FEATURE_ESTD_INTERFACE=${FEATURE_ESTD_INTERFACE}")

# Was strongly considering making parallel add_libraries here (-static and -interface flavors)
# but article [2] has changed my mind
if(FEATURE_ESTD_INTERFACE)
    add_library(${PROJECT_NAME} INTERFACE)

    # DEBT: direct interface compile of estd fails with this, though lib inclusion seems to work OK -
    # seeing as estd is always a lib, maybe that's a non issue.  Also, maybe all interface libs do that
    target_sources(${PROJECT_NAME} INTERFACE ${SOURCE_FILES})

    target_include_directories(${PROJECT_NAME} INTERFACE ${ESTD_INCLUDE_DIRECTORY})
    if(ESTD_OS_FREERTOS)
        message(DEBUG "estd: FreeRTOS compile defs")
        target_compile_definitions(${PROJECT_NAME} INTERFACE ESTD_OS_FREERTOS=${ESTD_OS_FREERTOS})
    endif()
else()
    add_library(${PROJECT_NAME} ${SOURCE_FILES})
    add_library(estd_lib ALIAS ${PROJECT_NAME})

    target_include_directories(${PROJECT_NAME} PUBLIC ${ESTD_INCLUDE_DIRECTORY})
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror=return-type)
endif()

add_library(malachi-iot::estd ALIAS ${PROJECT_NAME})

include(${TOOLS_DIR}/install.cmake)
