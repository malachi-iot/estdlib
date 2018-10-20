cmake_minimum_required(VERSION 3.1)

project("estdlib-unit-tests")

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(../../ext/Catch2/single_include/catch2)
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
    ios-test.cpp
    list-test.cpp
    map-test.cpp
    memory-test.cpp
    observer-test.cpp
    optional-test.cpp
    queue-test.cpp
    stack-test.cpp
    string-test.cpp
    test-data.h
    type-traits-test.cpp
    tuple-test.cpp
    vector-test.cpp
    priority-queue-test.cpp
    utility-test.cpp algorithm-test.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE UNIT_TESTING)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror=return-type)
# -Wextra) not ready for this just yet, but want it

target_link_libraries(${PROJECT_NAME} estd_lib)
