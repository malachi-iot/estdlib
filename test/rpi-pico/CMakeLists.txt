# This CMake is based on lessons from PGRPI-2

cmake_minimum_required(VERSION 3.13)

# initialize the SDK directly
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(rpi-unit-tests LANGUAGES C CXX)

set(ROOT_DIR ../..)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# picks up estd for us also
add_subdirectory(${ROOT_DIR}/test/unity unit-tests)

add_executable(${PROJECT_NAME}
    main.cpp
    proving.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE .)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(${PROJECT_NAME} pico_stdlib malachi-iot::estd unit-tests unity)
if(ESTD_OS_FREERTOS EQUAL 2)
    message(DEBUG "Linking against FreeRTOS")
    target_link_libraries(${PROJECT_NAME}
        #FreeRTOS-Kernel         # Static only
        FreeRTOS-Kernel-Heap4   # FreeRTOS kernel and dynamic heap
        )
endif()

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# wait indefinitely for a CDC to appear so we can see test results
# Beware -- reports indicate this hangs UART indefinitely
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=-1)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})