# ++ Experimental
# DEBT: I like 'define_property' better, but I can't quite get it into cache
option(ESTD_INTERFACE "Compile as an INTERFACE library")
define_property(GLOBAL PROPERTY FEATURE_ESTD_INTERFACE
        BRIEF_DOCS "When 1, estd compiles as an INTERFACE library"
        FULL_DOCS "See feature_flags.md for full description")
define_property(GLOBAL PROPERTY ESTD_OS_FREERTOS
        BRIEF_DOCS "When 1 (default) and FreeRTOS is detected, estd FreeRTOS support is enabled"
        FULL_DOCS "See feature_flags.md for full description")
# --

if(NOT DEFINED FEATURE_ESTD_FREERTOS)
    message(DEBUG "estd: defaulting FEATURE_ESTD_FREERTOS to 1")
    set(FEATURE_ESTD_FREERTOS 1)
endif()

# DEBT: Clean this to be just ESTD_INTERFACE
if(ESTD_INTERFACE)
    set(FEATURE_ESTD_INTERFACE 1)
endif()

if(PICO_SDK)
    include(${CMAKE_CURRENT_LIST_DIR}/rpi-pico.cmake)
endif()

if(FEATURE_ESTD_INTERFACE)
    set(ESTD_LINK_TYPE INTERFACE)
else()
    set(ESTD_LINK_TYPE PUBLIC)
endif()