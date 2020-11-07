cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_STANDARD 98)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

project(simple-tests)

set(UNITY_DIR ../../../ext/Unity)
set(ESTDLIB_DIR ../../../src)
set(TESTSRC_DIR ../../../test/unity)

add_subdirectory(${ESTDLIB_DIR} estdlib)
add_subdirectory(${UNITY_DIR} unity)

include_directories(${ESTDLIB_DIR})
include_directories(${TESTSRC_DIR})

set(TESTSRC
        ${TESTSRC_DIR}/align.cpp
        ${TESTSRC_DIR}/chrono.cpp
        ${TESTSRC_DIR}/cpp.cpp
        ${TESTSRC_DIR}/cstddef.cpp
        )

add_executable(${PROJECT_NAME}
        chrono-test.cpp
        "main.cpp"
        misc.cpp
        tuple-test.cpp
        ${TESTSRC}
        )

target_link_libraries(${PROJECT_NAME} unity)

