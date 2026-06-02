# Depends on setvars

find_package(Git)

string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

set(IN_DIR ${CMAKE_CURRENT_LIST_DIR}/in)
set(WORKING_DIR ${CMAKE_CURRENT_LIST_DIR}/temp)

message(DEBUG "version.cmake: ${CMAKE_CURRENT_SOURCE_DIR} / ${WORKING_DIR}")

if(Git_FOUND)
    add_custom_target(generate-version ALL
        COMMAND
            ${GIT_EXECUTABLE} describe --tags --always --dirty > ${WORKING_DIR}/git-described
        VERBATIM
    )
    FILE(READ ${WORKING_DIR}/git-described GIT_DESCRIBED)
    message(DEBUG "GIT_DESCRIBED: ${GIT_DESCRIBED}")

    set(SEMVER_REGEX "^v([0-9]+)\.([0-9]+)\.([0-9]+)?(-)?([0-9A-Za-z.-]+)")
    set(SEMVER_ID_REGEX "[0-9A-Za-z]+")

    string(REGEX MATCH ${SEMVER_REGEX} GIT_TAG_SEMVER ${GIT_DESCRIBED})
    #string(REGEX MATCHALL "^v([0-9]).([0-9])([.])?([0-9])?(-)?(.+)?" GIT_TAG_SEMVER ${GIT_DESCRIBED})

    set(ESTD_VER_MAJOR ${CMAKE_MATCH_1})
    set(ESTD_VER_MINOR ${CMAKE_MATCH_2})
    set(ESTD_VER_PATCH ${CMAKE_MATCH_3})
    set(ESTD_VER_SUFFIX ${CMAKE_MATCH_5})

    message(STATUS "estd: v${ESTD_VER_MAJOR}.${ESTD_VER_MINOR}.${ESTD_VER_PATCH} (${ESTD_VER_SUFFIX})")

    string(REGEX MATCHALL "${SEMVER_ID_REGEX}" GIT_TAG_SEMVER_ID "${ESTD_VER_SUFFIX}")

    list(GET GIT_TAG_SEMVER_ID 0 ESTD_SEMVER_ID)

    #foreach(w IN LISTS GIT_TAG_SEMVER_ID)
    #    message(STATUS "word: ${w}")
    #endforeach()
    message(STATUS "ESTD_SEMVER_ID: ${ESTD_SEMVER_ID}")

elseif()
    message(STATUS "Couldn't find git, skipping version header generation")
endif()

#add_custom_target(generate_version ALL
#    COMMAND
#        ${Python3_EXECUTABLE}
#        ${ROOT_DIR}/tools/python/git-describe-to-header.py > ${WORKING_DIR}/git-described.cmake
#    VERBATIM
#)


# DEBT: 'port' is not a great destination
configure_file(
    ${IN_DIR}/version.in.h
    ${CMAKE_CURRENT_SOURCE_DIR}/estd/port/version.h)

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
	