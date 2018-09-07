cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 98)

project(simple-tests)
add_executable(${PROJECT_NAME}
    chrono-test.cpp
    "main.cpp"
    misc.cpp
    )

set(ESTDLIB_DIR ../../../src)

add_subdirectory(${ESTDLIB_DIR} estdlib)
include_directories(${ESTDLIB_DIR})

