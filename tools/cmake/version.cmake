include(${CMAKE_CURRENT_LIST_DIR}/setvars.cmake)

set(IN_DIR ${CMAKE_CURRENT_LIST_DIR}/in)
set(WORKING_DIR ${CMAKE_CURRENT_LIST_DIR}/temp)

include(${CMAKE_CURRENT_LIST_DIR}/git-describe.cmake)
include(${WORKING_DIR}/git-described.cmake)

string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

message(DEBUG "version.cmake: ${CMAKE_CURRENT_SOURCE_DIR} / ${WORKING_DIR}")

# DEBT: 'port' is not a great destination
configure_file(
    ${IN_DIR}/version.in.h
    ${CMAKE_CURRENT_SOURCE_DIR}/estd/port/version.h)

configure_file(
    ${IN_DIR}/git-version.in.h
    ${CMAKE_CURRENT_SOURCE_DIR}/estd/port/git-version.h)

# esp-idf: For internal testing
configure_file(
    ${IN_DIR}/idf_component.in.yml
    ${ROOT_DIR}/tools/esp-idf/components/estd/idf_component.yml)

# esp-idf: For general use
configure_file(
    ${IN_DIR}/idf_component.in.yml
    ${ROOT_DIR}/idf_component.yml)

# For platformio
configure_file(
    ${IN_DIR}/library.in.json
    ${ROOT_DIR}/library.json)
	