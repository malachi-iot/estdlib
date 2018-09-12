cmake_minimum_required(VERSION 2.8)

project("estdlib-unit-tests")

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DUNIT_TESTING)

INCLUDE_DIRECTORIES(../../ext/Catch2/single_include)
set(ESTDLIB_DIR ../../src)

add_subdirectory(${ESTDLIB_DIR} estdlib)
include_directories(${ESTDLIB_DIR})

add_executable(${PROJECT_NAME} "main.cpp"
    allocator-test.cpp
    array-test.cpp
    buffer-test.cpp
    chrono-test.cpp
    experimental-test.cpp
    functional-test.cpp
    list-test.cpp
    map-test.cpp
    memory-test.cpp
    observer-test.cpp
    queue-test.cpp
    string-test.cpp
    test-data.h
    type-traits-test.cpp
    vector-test.cpp
    priority-queue-test.cpp
        ios-test.cpp utility-test.cpp)

target_link_libraries(${PROJECT_NAME} estd_lib)
