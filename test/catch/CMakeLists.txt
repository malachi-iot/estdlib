cmake_minimum_required(VERSION 3.12)

project("estd-unit-tests")

if(NOT DEFINED CMAKE_CXX_STANDARD)
    message(DEBUG "defaulting to c++11")
    set(CMAKE_CXX_STANDARD 11)
else()
    message(STATUS "using c++${CMAKE_CXX_STANDARD}")
endif()

include(../../tools/cmake/setvars.cmake)

INCLUDE_DIRECTORIES(${ROOT_DIR}/ext/Catch2/single_include/catch2)

add_subdirectory(${ESTD_DIR} estd)

add_executable(${PROJECT_NAME} "main.cpp"
    algorithm-test.cpp
    allocator-test.cpp
    array-test.cpp
    buffer-test.cpp
    c++03-test.cpp
    chrono-test.cpp
	cstddef-test.cpp
    experimental/memory-pool1-test.cpp
    experimental/memory-pool2-test.cpp
    experimental/memory-pool3-test.cpp
    experimental-test.cpp
    functional-test.cpp
    ios-test.cpp
    iostream-test.cpp
    istream-test.cpp
    iterator-test.cpp
    list-test.cpp
    limits-test.cpp
	locale-test.cpp
    map-test.cpp
    memory-test.cpp
    misc-test.cpp
    optional-test.cpp
    ostream-test.cpp
    priority-queue-test.cpp
    queue-test.cpp
    stack-test.cpp
	streambuf-test.cpp
    string-test.cpp
    ratio-test.cpp
    test-data.h
    type-traits-test.cpp
    tuple-test.cpp
    vector-test.cpp
    utility-test.cpp
        internal-test.cpp charconv-test.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE UNIT_TESTING)

# TODO: Instead, filter by presence of GCC
if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /Zc:__cplusplus)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror=return-type)

# -Wno-unused-variable want this too, remember .H files aren't compiled independently
#   so we have to report unused variables in unit tests fused with headers
# -Wextra) not ready for this just yet, but want it

target_link_libraries(${PROJECT_NAME} estd)
