# This file is designed to be included by the top-level project 
# CMakeLists.txt just before your main include, for example:

## include(${ESTDLIB_DIR}/tools/esp-idf/project.cmake)
## include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# This explicit include is necessary instead of a regular component
# so that version_finder gets picked up.  Additionally, we set
# ESTDLIB_DIR for convenience

# Somehow, NO_POLICY_SCOPE style behavior appears to work here,
# so ESTDLIB_DIR and the variables set in version_finder.cmake do
# propagate upward


set(EXTRA_COMPONENT_DIRS
    "${EXTRA_COMPONENT_DIRS}"
    "${CMAKE_CURRENT_LIST_DIR}/components"
    )

set(ESTDLIB_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
get_filename_component(ESTDLIB_DIR ${ESTDLIB_DIR} ABSOLUTE)

#message("ESTDLIB building: ${ESTDLIB_DIR}")

include(${CMAKE_CURRENT_LIST_DIR}/version_finder.cmake)
