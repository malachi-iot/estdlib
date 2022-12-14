# If we're using FreeRTOS, we have to be an INTERFACE library so that #includes of FreeRTOSConfig.h
# or other incidental FreeRTOS headers from estd's cpp files succeeds.  This also alleviates the
# need to specify FreeRTOS linkage.
#
# If we're not using FreeRTOS, a regular static compilation works fine.
#
# DEBT: estd's few cpp files probably don't truly care about FreeRTOS, so it may be possible to filter
# at that level.  Still, rpi pico seems to favor INTERFACE libraries anyway.

if(DEFINED ENV{FREERTOS_KERNEL_PATH} AND FEATURE_ESTD_FREERTOS)
    message(STATUS "estd: Raspberry Pi Pico (FreeRTOS)")

    set(FEATURE_ESTD_INTERFACE 1)

    # Tested against v10.5.1
    include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

    # '2' here means flag as FREERTOS but then rewrite as 1 or RTOS version number
    # in our version.h
    # DEBT: Brute force, would prefer to acquire property from estd scope
    # Interface properties present special challenges
    # https://stackoverflow.com/questions/68502038/custom-properties-for-interface-libraries
    set(ESTD_OS_FREERTOS 2 CACHE INTERNAL "")

    # This is set up in main estd CMakeLists.txt now
    #add_compile_definitions(ESTD_OS_FREERTOS=${ESTD_OS_FREERTOS})
else()
    message(STATUS "estd: Raspberry Pi Pico (bare metal)")

    # DEBT: Kind of obnoxious, have to forcefully remove it because, well
    # it IS cached...
    unset(ESTD_OS_FREERTOS CACHE)
endif()
