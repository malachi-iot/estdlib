if(NOT DEFINED FEATURE_ESTD_FREERTOS)
    message(DEBUG "estdlib: defaulting FEATURE_ESTD_FREERTOS to 1")
    set(FEATURE_ESTD_FREERTOS 1)
endif()

if(PICO_SDK)
    include(${CMAKE_CURRENT_LIST_DIR}/rpi-pico.cmake)
endif()