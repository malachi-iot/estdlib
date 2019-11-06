set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
#set(ROOT_DIR $ENV{ESTDLIB_DIR})

set(COMPONENT_ADD_INCLUDEDIRS 
    ${COMPONENT_ADD_INCLUDEDIRS}
    ${ROOT_DIR}/src)

#include(${CMAKE_CURRENT_LIST_DIR}/version_finder.cmake)

register_component()