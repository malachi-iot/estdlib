# Guidance from
# https://stackoverflow.com/questions/31546278/where-to-set-cmake-configuration-types-in-a-project-with-subprojects
# Eventual goal is to multi target to c++11, c++14, c++20, etc. c++03/98 will still be targeted in a separate
# Project due to catch.hpp vs catch2.hpp

# Be sure to move this out into the /tools folder (maybe /tools/cmake)

# Interesting tidbits here:
# https://youtrack.jetbrains.com/issue/CPP-19478 (from Ivan K, no less)

if(NOT SET_UP_CONFIGURATIONS_DONE)
    set(SET_UP_CONFIGURATIONS_DONE TRUE)

    # No reason to set CMAKE_CONFIGURATION_TYPES if it's not a multiconfig generator
    # Also no reason mess with CMAKE_BUILD_TYPE if it's a multiconfig generator.
    get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(${isMultiConfig} OR ("$ENV{CLION_IDE}" STREQUAL "TRUE"))
        message("Multiconfig got here 1")
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile;c++20" CACHE STRING "" FORCE)
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
endif()