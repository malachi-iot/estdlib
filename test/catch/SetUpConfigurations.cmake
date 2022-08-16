# Guidance from
# https://stackoverflow.com/questions/31546278/where-to-set-cmake-configuration-types-in-a-project-with-subprojects
# Eventual goal is to multi target to c++11, c++14, c++20, etc. c++03/98 will still be targeted in a separate
# Project due to catch.hpp vs catch2.hpp

# Be sure to move this out into the /tools folder (maybe /tools/cmake)

# Interesting tidbits here:
# https://youtrack.jetbrains.com/issue/CPP-19478 (from Ivan K, no less)

# It's starting to seem like it's not possible to (smoothly) set C++ version compliance
# via the CONFIG/profile CMake modes:
# https://discourse.cmake.org/t/using-get-target-property-or-just-get-property-with-generator-expressions/2090/6
# https://stackoverflow.com/questions/66037258/cmake-generator-expression-in-a-target-property-is-not-evaluated
# https://stackoverflow.com/questions/55262774/cmake-set-property-command-with-generator-expressions-using-multiple-values
# https://gitlab.kitware.com/cmake/cmake/-/issues/17611

if(NOT SET_UP_CONFIGURATIONS_DONE)
    set(SET_UP_CONFIGURATIONS_DONE TRUE)

    # No reason to set CMAKE_CONFIGURATION_TYPES if it's not a multiconfig generator
    # Also no reason mess with CMAKE_BUILD_TYPE if it's a multiconfig generator.
    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(${isMultiConfig} OR ("$ENV{CLION_IDE}" STREQUAL "TRUE"))
        message("Multiconfig got here 1")
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release;c++14;c++20" CACHE STRING "" FORCE)
    else()
        message("Multiconfig got here 2")
        if(NOT CMAKE_BUILD_TYPE)
            message("Defaulting to release build.")
            set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
        endif()
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
        # set the valid options for cmake-gui drop-down list
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;Profile")
    endif()

    # None of the below are functional.  It seems CMake generators may be a 2nd pass
    # and not available during traditional CMake scan/building
    # https://stackoverflow.com/questions/51353110/how-do-i-output-the-result-of-a-generator-expression-in-cmake
    # If so, then we may need to set the C++ language version with a cascading generator if statement of
    # sorts

    set(TEST123 $<CONFIG>)

    message($<C_COMPILER_VERSION>)
    message($<CONFIG:Debug>)
    message("Testing: $<CONFIG> ${TEST123}")

    set(TEST_FILE "log.txt")

    # add_custom_command does not create a new target. You have to define targets explicitly
    # by add_executable, add_library or add_custom_target in order to make them visible to make
    add_custom_command(OUTPUT ${TEST_FILE}
            COMMAND touch ${TEST_FILE}

            # Display the given message before the commands are executed at build time
            COMMENT "Creating ${TEST_FILE}"
            )

    add_custom_target(print_config ALL
            # Prints "Config is Debug" in this single-config case
            COMMAND ${CMAKE_COMMAND} -E echo "Config is $<CONFIG>"
            VERBATIM
            )

    if("$<CONFIG:c++20>")
        message("c++20 mode")
        set(ESTD_CXX_STANDARD 20)
    elseif("$<CONFIG:c++14>")
        message("c++14 mode")
        set(ESTD_CXX_STANDARD 14)
    else()
        message("c++11 mode")
        set(ESTD_CXX_STANDARD 11)
    endif()
endif()