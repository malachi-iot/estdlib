# Depends on setvars

string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

set(WORKING_DIR ${CMAKE_CURRENT_LIST_DIR}/in)

# DEBT: 'port' is not a great destination
configure_file(
    ${WORKING_DIR}/version.in.h
    ${CMAKE_CURRENT_SOURCE_DIR}/estd/port/version.h)

configure_file(
    ${WORKING_DIR}/idf_component.in.yml
    ${ROOT_DIR}/tools/esp-idf/components/estdlib/idf_component.yml)

# For platformio
configure_file(
    ${WORKING_DIR}/library.in.json
    ${ROOT_DIR}/library.json)
	