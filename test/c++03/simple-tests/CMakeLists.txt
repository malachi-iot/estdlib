cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 98)

# DEBT: This is not always true for C++98 scenarios, but implicitly true for ADSPBFIN (inttypes.h isn't
# available, but it seems to have those defs)
add_compile_definitions(FEATURE_STD_INTTYPES)

project(simple-tests)

set(UNITY_DIR ../../../ext/Unity)
set(ESTDLIB_DIR ../../../src)
set(TESTSRC_DIR ../../../test/unity)

add_subdirectory(${ESTDLIB_DIR} estdlib)
add_subdirectory(${UNITY_DIR} unity)

include_directories(${TESTSRC_DIR})

set(TESTSRC
        ${TESTSRC_DIR}/align.cpp
        ${TESTSRC_DIR}/array.cpp
        ${TESTSRC_DIR}/chrono.cpp
        ${TESTSRC_DIR}/cpp.cpp
        ${TESTSRC_DIR}/cstddef.cpp
        ${TESTSRC_DIR}/limits.cpp
        ${TESTSRC_DIR}/locale.cpp
        ${TESTSRC_DIR}/optional.cpp
        ${TESTSRC_DIR}/queue.cpp
        ${TESTSRC_DIR}/ratio.cpp
        ${TESTSRC_DIR}/span.cpp
        ${TESTSRC_DIR}/streambuf.cpp
        ${TESTSRC_DIR}/string.cpp
        ${TESTSRC_DIR}/tuple.cpp
        )

#   ${TESTSRC_DIR}/map.cpp
#   ${TESTSRC_DIR}/thread.cpp

add_executable(${PROJECT_NAME}
        "main.cpp"
        misc.cpp
        tuple-test.cpp
        ${TESTSRC}
        )

target_link_libraries(${PROJECT_NAME} unity estd_lib)

