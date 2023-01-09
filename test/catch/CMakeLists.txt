cmake_minimum_required(VERSION 3.1)

project("estd-unit-tests")

set(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(../../ext/Catch2/single_include/catch2)
set(ESTD_DIR ../../src)
get_filename_component(ROOT_DIR ../.. ABSOLUTE)

include(ExternalProject)

# As per
# https://stackoverflow.com/questions/15175318/cmake-how-to-build-external-projects-and-include-their-targets
# https://mirkokiefer.com/cmake-by-example-f95eb47d45b1
# It seems we can't use the regular add_subdirectory which means we have to manually specify 'include_directories'
# etc. yuck.

ExternalProject_Add(Catch2_ext
        GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
        GIT_TAG "v2.13.10"
        INSTALL_DIR ${ROOT_DIR}/ext/cmake/install/catch
        SOURCE_DIR ${ROOT_DIR}/ext/cmake/src/catch
        BINARY_DIR ${ROOT_DIR}/ext/cmake/bin
        PREFIX ${ROOT_DIR}/ext/cmake/prefix/catch
        #CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:<INSTALL_DIR>
        #BUILD_COMMAND ""
        INSTALL_COMMAND ""
        )

ExternalProject_Get_Property(Catch2_ext install_dir)

add_subdirectory(${ESTD_DIR} estd)
#add_subdirectory(${ROOT_DIR}/ext/cmake/src/catch catch)

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

add_dependencies(${PROJECT_NAME} Catch2_ext)
add_library(Catch2_test STATIC IMPORTED)
add_dependencies(Catch2_test Catch2_ext)
set_property(TARGET Catch2_test PROPERTY IMPORTED_LOCATION ${install_dir})
#add_subdirectory(${ROOT_DIR}/ext/cmake/src/catch Catch2_)

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

target_link_libraries(${PROJECT_NAME} estd Catch2 Catch2_test)
