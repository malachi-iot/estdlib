# Raspberry Pi Pico Unit Testing

## FreeRTOS

Config file comes from https://github.com/raspberrypi/pico-examples/blob/sdk-1.4.0/pico_w/freertos/ping/FreeRTOSConfig.h

## Results

|   Date  | Board                | PICO_SDK | FreeRTOS | Result | Notes |
| ------- | -------------------- | -------  | -------- | ------ | ----- |
| 29NOV22 | Raspberry Pi Pico W  | v1.4.0   |  10.5.1  | Pass   | 
| 29NOV22 | Raspberry Pi Pico W  | v1.4.0   |  none    | Fail   | std::chrono::system_clock might depend on FreeRTOS
