# platformio unit tests

Shared unity tests with esp32 rtos + visualdsp

> These tests are incompatible with platformio 6.0+
> It remains a mystery how to encourage platformio to see unity files
> located outside the project folder (in this case, `../../unity`)


## Results

|   Date  | Vendor    | Platform      | Result |
| ------- | --------- | --------------| ------ |
| 19MAY22 | Microchip | ARM SAMD21 m0 | Pass   |
| 19MAY22 | Microchip | AVR 32u4      | Pass   |
| 20APR22 | Espressif | ESP32S        | Pass   |
