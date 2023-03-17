# Raspberry Pi Pico Unit Testing

## FreeRTOS

Config file comes from https://github.com/raspberrypi/pico-examples/blob/sdk-1.4.0/pico_w/freertos/ping/FreeRTOSConfig.h

Be sure to clone FreeRTOS itself from https://github.com/FreeRTOS/FreeRTOS-Kernel

Be sure to set FREERTOS_KERNEL_PATH as per https://github.com/racka98/PicoW-FreeRTOS-Template

## Results

|   Date  | Board                | PICO_SDK     | FreeRTOS | Result  | Notes |
| ------- | -------------------- | ------------ | -------- | -------- | ----- |
| 17MAR23 | Raspberry Pi Pico W  | v1.5.0       |  10.5.1  | Pass     | FreeRTOS dynamic only
| 01DEC22 | Raspberry Pi Pico W  | v1.4.0       |  10.5.1  | Pass     | FreeRTOS static + dynamic
| 16JAN23 | Raspberry Pi Pico W  | v1.4.0 (dev) |  none    | Pass     | latest `develop` branch 488bd
| 17MAR23 | Raspberry Pi Pico    | v1.5.0       |  none    | Compiles | Compiled on ARM64 platform, no rpi pico present
