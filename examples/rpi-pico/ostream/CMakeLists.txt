cmake_minimum_required(VERSION 3.13)

# initialize the SDK directly
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(ostream-example LANGUAGES C CXX)

set(ROOT_DIR ../../..)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Forcefully disable any FreeRTOS inclusion.  Not necessary for most
# people as the system usually auto deduces whether you want it.
set(FEATURE_ESTD_FREERTOS 0)

add_subdirectory(${ROOT_DIR}/src estd)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} pico_stdlib estd)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# wait indefinitely for a CDC to appear so we can see test results
# Beware -- reports indicate this hangs UART indefinitely
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS=-1)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})

