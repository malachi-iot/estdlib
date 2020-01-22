set(EXTRA_COMPONENT_DIRS
    "${EXTRA_COMPONENT_DIRS}"
    "${CMAKE_CURRENT_LIST_DIR}/components"
    )

set(ESTDLIB_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
get_filename_component(ESTDLIB_DIR ${ESTDLIB_DIR} ABSOLUTE)

#message("ESTDLIB building: ${ESTDLIB_DIR}")

include(${CMAKE_CURRENT_LIST_DIR}/version_finder.cmake)
