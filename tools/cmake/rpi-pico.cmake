if(DEFINED ENV{FREERTOS_KERNEL_PATH} AND FEATURE_ESTD_FREERTOS)
    message(STATUS "estd: Raspberry Pi Pico (FreeRTOS)")

    # Doing this so that FreeRTOS linkage can be specified in
    # consuming project
    set(FEATURE_ESTD_INTERFACE 1)

    # Tested against v10.5.1
    include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

    # DEBT: '2' here means flag as FREERTOS but then rewrite as 1 or 
    # RTOS version number - not so clear
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
