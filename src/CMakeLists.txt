# All references see /docs/references.md

cmake_minimum_required(VERSION 3.13)

project(estd VERSION 0.8.3 LANGUAGES CXX
    DESCRIPTION "Embedded oriented std library"
    HOMEPAGE_URL "https://github.com/malachi-iot/estdlib")

include(../tools/cmake/setvars.cmake)

include(${TOOLS_DIR}/platform.cmake)
include(${TOOLS_DIR}/includes.cmake)
include(${TOOLS_DIR}/ext.cmake)
include(${TOOLS_DIR}/version.cmake)
include(sources.cmake)

set(SOURCE_FILES ${SOURCE_FILES}
    internal/utility.cpp
    port/posix/streambuf.cpp
    )

list(TRANSFORM SOURCE_FILES PREPEND estd/)


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
    # DEBT: Strongly consider making this STATIC all the time (rather than implicitly SHARED)
    # since actual .cpp content is rather small
    add_library(${PROJECT_NAME} ${SOURCE_FILES})
    add_library(estd_lib ALIAS ${PROJECT_NAME})

    target_include_directories(${PROJECT_NAME} PUBLIC ${ESTD_INCLUDE_DIRECTORY})

    estd_compile_options()
endif()

add_library(malachi-iot::estd ALIAS ${PROJECT_NAME})

if(ESTD_DRAGONBOX)
    # DEBT: Discrepancy between FEATURE_ESTD_DRAGONBOX and ESTD_DRAGONBOX convention
    target_compile_definitions(${PROJECT_NAME} ${ESTD_LINK_TYPE} -DFEATURE_ESTD_DRAGONBOX=1)
    target_link_libraries(${PROJECT_NAME} dragonbox::dragonbox)
endif()

include(${TOOLS_DIR}/install.cmake)
