get_filename_component(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)
get_filename_component(TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR} ABSOLUTE)
set(ESTD_DIR ${ROOT_DIR}/src)

include(${TOOLS_DIR}/fetchcontent.cmake)
