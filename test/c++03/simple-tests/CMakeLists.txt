cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 98)

# DEBT: This is not always true for C++98 scenarios, but implicitly true for ADSPBFIN (inttypes.h isn't
# available, but it seems to have those defs)
add_compile_definitions(FEATURE_STD_INTTYPES)

project(estd-unity-tests)

set(UNITY_DIR ../../../ext/Unity)
set(ESTDLIB_DIR ../../../src)
set(TESTSRC_DIR ../../../test/unity)

add_subdirectory(${TESTSRC_DIR} src)

include_directories(${TESTSRC_DIR})

add_executable(${PROJECT_NAME}
        "main.cpp"
        tuple-test.cpp
        )

target_link_libraries(${PROJECT_NAME} unity estd unit-tests)

