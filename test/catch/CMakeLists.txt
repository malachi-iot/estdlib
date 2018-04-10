cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(../../ext/Catch2/single_include)
set(TARGET_PROJ_DIR ../../src)

add_subdirectory(${TARGET_PROJ_DIR} CMAKE_RUNTIME_OUTPUT_DIRECTORY)

project(estdlib-unittests)

add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} estdlib_lib)
