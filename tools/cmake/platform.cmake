# ++ Experimental
define_property(GLOBAL PROPERTY FEATURE_ESTD_INTERFACE)
define_property(GLOBAL PROPERTY ESTD_OS_FREERTOS)
# --

if(NOT DEFINED FEATURE_ESTD_FREERTOS)
    message(DEBUG "estd: defaulting FEATURE_ESTD_FREERTOS to 1")
    set(FEATURE_ESTD_FREERTOS 1)
endif()

if(PICO_SDK)
    include(${CMAKE_CURRENT_LIST_DIR}/rpi-pico.cmake)
endif()