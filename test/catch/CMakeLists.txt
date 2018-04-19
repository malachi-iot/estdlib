cmake_minimum_required(VERSION 2.8)

project("estdlib-unit-tests")

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(../../ext/Catch2/single_include)
set(TARGET_PROJ_DIR ../../src)

add_subdirectory(${TARGET_PROJ_DIR} CMAKE_RUNTIME_OUTPUT_DIRECTORY)
include_directories(${TARGET_PROJ_DIR})

add_executable(${PROJECT_NAME} "main.cpp"
    array-test.cpp
    experimental-test.cpp
    list-test.cpp
    map-test.cpp
    queue-test.cpp
    string-test.cpp
    )

target_link_libraries(${PROJECT_NAME} estd_lib)
