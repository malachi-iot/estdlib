set(ESTDLIB_DIR ../../../..)
#set(ESTDLIB_DIR "dummy")

# DEBT: This line is not touching ESTDLIB_DIR at all
get_filename_component(ESTDLIB_DIR,
    "${CMAKE_CURRENT_LIST_DIR}/${ESTDLIB_DIR}"
    ABSOLUTE)

#message("ESTDLIB building: ${ESTDLIB_DIR}")

include(${ESTDLIB_DIR}/tools/esp-idf/project.cmake)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

