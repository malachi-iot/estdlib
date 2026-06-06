include(${CMAKE_CURRENT_LIST_DIR}/setvars.cmake)

set(IN_DIR ${CMAKE_CURRENT_LIST_DIR}/in)

# 03JUN26 MB DEBT: Make this optional and inspect Python3_Interpreter_FOUND
find_package(Python3 COMPONENTS Interpreter)

# Was pretty promising but it occurs that all this file/regex stuff only really happens
# at configure time
#[[
find_package(Git)

if(Git_FOUND)
    add_custom_target(generate-version ALL
        COMMAND
            ${GIT_EXECUTABLE} describe --tags --always --dirty > ${WORKING_DIR}/git-described
        VERBATIM
    )
    file(READ ${WORKING_DIR}/git-described GIT_DESCRIBED)
    string(STRIP ${GIT_DESCRIBED} GIT_DESCRIBED)
    message(DEBUG "GIT_DESCRIBED: ${GIT_DESCRIBED}")

    set(SEMVER_REGEX "^v([0-9]+)\.([0-9]+)\.([0-9]+)?(-)?([0-9A-Za-z.-]+)")
    set(SEMVER_ID_REGEX "[0-9A-Za-z]+")

    string(REGEX MATCH ${SEMVER_REGEX} GIT_TAG_SEMVER ${GIT_DESCRIBED})
    #string(REGEX MATCHALL "^v([0-9]).([0-9])([.])?([0-9])?(-)?(.+)?" GIT_TAG_SEMVER ${GIT_DESCRIBED})

    set(GIT_TAG_SEMVER_MAJOR ${CMAKE_MATCH_1})
    set(GIT_TAG_SEMVER_MINOR ${CMAKE_MATCH_2})
    set(GIT_TAG_SEMVER_PATCH ${CMAKE_MATCH_3})
    set(GIT_TAG_SEMVER_SUFFIX ${CMAKE_MATCH_5})

    message(STATUS "estd: v${GIT_TAG_SEMVER_MAJOR}.${GIT_TAG_SEMVER_MINOR}.${GIT_TAG_SEMVER_PATCH} (${GIT_TAG_SEMVER_SUFFIX})")

    string(REGEX MATCHALL "${SEMVER_ID_REGEX}" GIT_TAG_SEMVER_SUFFIX2 "${GIT_TAG_SEMVER_SUFFIX}")

    list(GET GIT_TAG_SEMVER_SUFFIX2 0 GIT_TAG_SEMVER_ID)

    #foreach(w IN LISTS GIT_TAG_SEMVER_SUFFIX)
    #    message(STATUS "word: ${w}")
    #endforeach()
    message(STATUS "GIT_TAG_SEMVER_ID: ${GIT_TAG_SEMVER_ID}")

elseif()
    message(STATUS "Couldn't find git, skipping version header generation")
    set(GIT_TAG_SEMVER_ID "")
endif()
]]

if(Python3_Interpreter_FOUND)
    add_custom_target(generate-version ALL
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        COMMAND
            Python3::Interpreter ../python/git-describe-to-header.py
                estd
                ${IN_DIR}/git-version.in.h >
                ${ROOT_DIR}/src/estd/port/git-version.h
        VERBATIM
    )
else()
    message(STATUS "Python3 not found: skipping git-version.h generation")
endif()