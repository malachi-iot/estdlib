cmake_minimum_required(VERSION 3.13)

project(unit-tests LANGUAGES C CXX)

set(ROOT_DIR ../..)

add_subdirectory(${ROOT_DIR}/src estd)
add_subdirectory(${ROOT_DIR}/ext/Unity unity)

set(SOURCES
    align.cpp
    array.cpp
    #chrono.cpp
    cpp.cpp
    cstddef.cpp
    functional.cpp
    limits.cpp
    #locale.cpp
    map.cpp
    optional.cpp
    queue.cpp
    ratio.cpp
    span.cpp
    #streambuf.cpp
    string.cpp
    #thread.cpp
    tuple.cpp
    )

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_link_libraries(${PROJECT_NAME} unity estd)
