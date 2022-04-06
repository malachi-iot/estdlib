cmake_minimum_required(VERSION 3.1)

project("estdlib-unit-tests")

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(../../ext/Catch2/single_include/catch2)
set(ESTDLIB_DIR ../../src)

add_subdirectory(${ESTDLIB_DIR} estdlib)

add_executable(${PROJECT_NAME} "main.cpp"
    allocator-test.cpp
    array-test.cpp
    buffer-test.cpp
    chrono-test.cpp
	cstddef-test.cpp
    experimental-test.cpp
    functional-test.cpp
    ios-test.cpp
    iostream-test.cpp
    iterator-test.cpp
    list-test.cpp
    limits-test.cpp
	locale-test.cpp
    map-test.cpp
    memory-test.cpp
    optional-test.cpp
    queue-test.cpp
    stack-test.cpp
    string-test.cpp
    test-data.h
    type-traits-test.cpp
    tuple-test.cpp
    vector-test.cpp
    priority-queue-test.cpp
    utility-test.cpp
	algorithm-test.cpp
        internal-test.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE UNIT_TESTING)
# TODO: Instead, filter by presence of GCC
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall /Zc:__cplusplus)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror=return-type)
endif()
# -Wno-unused-variable want this too, remember .H files aren't compiled independently
#   so we have to report unused variables in unit tests fused with headers
# -Wextra) not ready for this just yet, but want it

target_link_libraries(${PROJECT_NAME} estd_lib)
