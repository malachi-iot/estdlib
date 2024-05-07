cmake_minimum_required(VERSION 3.13)

project(unit-tests LANGUAGES C CXX)

include(../../tools/cmake/setvars.cmake)

Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    Unity
    GIT_REPOSITORY  https://github.com/ThrowTheSwitch/Unity
    GIT_TAG         v2.5.2
    GIT_PROGRESS    TRUE
    GIT_SHALLOW     TRUE
)

FetchContent_MakeAvailable(Unity)

add_subdirectory(${ROOT_DIR}/src estd)

set(SOURCES
    align.cpp
    array.cpp
    bipbuf.cpp
    chrono.cpp
    cpp.cpp
    cstddef.cpp
    expected.cpp
    functional.cpp
    housekeeping.cpp
    limits.cpp
    locale.cpp
    optional.cpp
    ostream.cpp
    queue.cpp
    ratio.cpp
    span.cpp
    streambuf.cpp
    string.cpp
    tuple.cpp
    variant.cpp
    )

message(DEBUG "estd unity: CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

if(NOT CMAKE_CXX_STANDARD EQUAL 98)
    set(SOURCES ${SOURCES}
        map.cpp
        thread.cpp
        )
endif()

# NOTE: Making this an INTERFACE library so that flags and compiler settings
# of parent projects are picked up during compilation
add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME} INTERFACE ${SOURCES})

target_include_directories(${PROJECT_NAME} INTERFACE .)

target_link_libraries(${PROJECT_NAME} INTERFACE unity estd)
