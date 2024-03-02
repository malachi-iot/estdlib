# Is used by, and is independent of, our project.cmake

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

# 'sets' won't propagate upward even with NO_POLICY_SCOPE because
# components are handled as a specially isolated module in esp-idf
#set(ESTDLIB_DIR ${ROOT_DIR})

# DEBT: Use list(PREPEND) here and consolidate into sources.cmake
set(COMPONENT_SRCS 
    ${ROOT_DIR}/src/estd/ext/willemt/bipbuffer/bipbuffer.c
    ${ROOT_DIR}/src/estd/internal/string_convert.cpp)

set(COMPONENT_ADD_INCLUDEDIRS 
    ${ROOT_DIR}/src)

# NOTE: Even though we do this, a special cmake_policy or similar
# must be enacted for these changes to cascade out to 'includers',
# so for the time being you'll need to expressly include 
# version_finder.cmake yourself in your own CMakeLists.txt
# This is most easily done by using project.cmake which also auto
# configures this component for use
include(${CMAKE_CURRENT_LIST_DIR}/version_finder.cmake)

register_component()

# For diagnostic only - will break compilation
#component_compile_options("-Wstack-usage=32" "-S")