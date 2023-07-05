string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

# DEBT: 'port' is not a great destination
configure_file(
	${CMAKE_CURRENT_LIST_DIR}/version.in.h
	${CMAKE_CURRENT_SOURCE_DIR}/estd/port/version.h)

