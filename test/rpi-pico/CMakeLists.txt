# This CMake is based on lessons from PGRPI-2

cmake_minimum_required(VERSION 3.13)

# initialize the SDK directly
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(rpi-unit-tests LANGUAGES C CXX)

set(ROOT_DIR ../..)

add_subdirectory(${ROOT_DIR}/test/unity unit-tests)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

if(DEFINED ENV{FREERTOS_KERNEL_PATH})
    message(STATUS "GOT HERE=${PICO_SDK}")
    include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
endif()

add_executable(${PROJECT_NAME}
    main.cpp
    proving.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE .)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(${PROJECT_NAME} pico_stdlib malachi-iot::estd unit-tests unity)
target_link_libraries(${PROJECT_NAME} FreeRTOS-Kernel)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})