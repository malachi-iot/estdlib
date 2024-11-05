set(ROOT_DIR ../../../..)
#set(ESTDLIB_DIR "dummy")

# DEBT: 23JUL23 Fixed this line but haven't tested yet
get_filename_component(ESTDLIB_DIR
    "${CMAKE_CURRENT_LIST_DIR}/${ROOT_DIR}"
    ABSOLUTE)

#message("ESTDLIB building: ${ROOT_DIR}")

include(${ROOT_DIR}/tools/esp-idf/project.cmake)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

