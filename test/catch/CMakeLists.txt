cmake_minimum_required(VERSION 3.12)

project("estd-unit-tests" VERSION 1.0)

set(FEATURE_ESTD_COMPILE_STRICTNESS 4 CACHE STRING
    "TBD")
set(FEATURE_ESTD_COMPILE_VERBOSITY 3 CACHE STRING
    "TBD")

if(NOT DEFINED CMAKE_CXX_STANDARD)
    message(DEBUG "defaulting to c++11")
    set(CMAKE_CXX_STANDARD 11)
else()
    message(STATUS "using c++${CMAKE_CXX_STANDARD}")
endif()

include(../../tools/cmake/setvars.cmake)

Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY  https://github.com/catchorg/Catch2.git
    GIT_TAG         v2.13.10
    GIT_PROGRESS    TRUE
    GIT_SHALLOW     TRUE
)

FetchContent_MakeAvailable(Catch2)

message(DEBUG "Source dir of Catch2 = ${Catch2_SOURCE_DIR}")

# DEBT: Include <catch2/catch.hpp> directly and don't rely on this
INCLUDE_DIRECTORIES(${Catch2_SOURCE_DIR}/include)

add_subdirectory(${ROOT_DIR} estd)

set(EXTRA_FILES
    ${ROOT_DIR}/README.md
    ${ROOT_DIR}/CHANGELOG.md
    ${ROOT_DIR}/docs/Variadic.md
    ${ROOT_DIR}/docs/Units.md
)

add_executable(${PROJECT_NAME} "main.cpp" ${EXTRA_FILES})

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/sources.cmake)
    # Since unit tests take a while to compile now, have an override
    # sources.cmake (excluded from source control) for spot targeted
    # testing
    include(sources.cmake)
else()
    target_sources(${PROJECT_NAME} PRIVATE
        algorithm-test.cpp
        allocator-test.cpp
        array-test.cpp
        bit-test.cpp
        buffer-test.cpp
        c++03-test.cpp
        charconv-test.cpp
        chrono-test.cpp
        cstddef-test.cpp
        dynamic-array-test.cpp
        flags-test.cpp
        experimental/memory-pool1-test.cpp
        experimental/memory-pool2-test.cpp
        experimental/memory-pool3-test.cpp
        expected-test.cpp
        experimental-test.cpp
        functional-test.cpp
        ios-test.cpp
        iostream-test.cpp
        istream-test.cpp
        iterator-test.cpp
        list-test.cpp
        limits-test.cpp
        locale-test.cpp
        macros-test.cpp
        map-test.cpp
        memory-pool-test.cpp
        memory-test.cpp
        misc-test.cpp
        optional-test.cpp
        ostream-test.cpp
        port-test.cpp
        priority-queue-test.cpp
        queue-test.cpp
        stack-test.cpp
        streambuf-test.cpp
        string-test.cpp
        string-view-test.cpp
        ratio-test.cpp
        test-data.h
        type-traits-test.cpp
        tuple-test.cpp
        units-test.cpp
        utility-test.cpp
        variadic-test.cpp
        variant-test.cpp
        vector-test.cpp
        internal-test.cpp)
endif()

# DEBT: Make a linkage property so that we don't have to double these guys up
if(FEATURE_ESTD_INTERFACE)
    target_compile_definitions(estd INTERFACE
        FEATURE_ESTD_COMPILE_VERBOSITY=${FEATURE_ESTD_COMPILE_VERBOSITY}
        FEATURE_ESTD_COMPILE_STRICTNESS=${FEATURE_ESTD_COMPILE_STRICTNESS}
    )
else()
    target_compile_definitions(estd PUBLIC
        FEATURE_ESTD_COMPILE_VERBOSITY=${FEATURE_ESTD_COMPILE_VERBOSITY}
        FEATURE_ESTD_COMPILE_STRICTNESS=${FEATURE_ESTD_COMPILE_STRICTNESS}
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE UNIT_TESTING)

estd_compile_options()

target_link_libraries(${PROJECT_NAME} estd Catch2::Catch2)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/contrib)

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
