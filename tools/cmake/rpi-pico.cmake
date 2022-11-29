message(DEBUG "estdlib: Raspberry Pi Pico mode")

set(ESTD_AS_INTERFACE 1)

if(DEFINED ENV{FREERTOS_KERNEL_PATH})
    # Tested against v10.5.1
    include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
    # DEBT: '2' here means flag as FREERTOS but then rewrite as 1 or 
    # RTOS version number - not so clear
    add_compile_definitions(ESTD_OS_FREERTOS=2)
    set(ESTD_OS_FREERTOS 2)
endif()
