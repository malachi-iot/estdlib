Invoke just basic compliation for Arduino targets to ensure it indeed works

## Results

| Project     |   Date  | Vendor    | Platform      | Result   | Notes |
| ----------- | ------- | --------- | ------------- | -------- | -----
| basic       | 27JUN23 | Microchip | ARM SAMD21 m0 | Partial  | Nearly passes see Footnote [2]
| basic       | 27JUN23 | Microchip | AVR 32u4      | Fail     | std::chrono::steady_time not available
| basic       | 27JUN23 | Microchip | ARM SAM m4    | Compiles |
| from_chars  | 06MAY22 | Microchip | ARM SAMD21 m0 | Pass   |
| from_chars  | 19APR22 | Microchip | AVR 32u4      | Pass   |
| from_chars  | 20APR22 | Espressif | ESP32S        | Pass   |

| ostream     | 27JUN23 | Microchip | ARM SAMD21 m0 | Pass     |
| ostream     | 27JUN23 | Microchip | AVR 32u4      | Compiles |
| ostream     | 27JUN23 | Espressif | ESP32         | Compiles |
| ostream     | 27JUN23 | Espressif | ESP32C3       | Fail     | pio can't find compiler package
| ostream     | 27JUN23 | STM       | STM32F303K8   | Compiles |
| ostream     | 27JUN23 | Microchip | ATtiny        | Compiles |
| unity       | 27JUN23 | Microchip | ARM SAMD21 m0 | Pass     |
| unity       | 27JUN23 | Microchip | ARM SAMD51 m4 | Compiles |
| unity       | 27JUN23 | Microchip | AVR 32u4      | Fail     |
| unity       | 27JUN23 | Espressif | ESP32S        | Compiles |
| unity       | 27JUN23 | Espressif | ESP32C3       | Fail     | pio can't find compiler package
| unity       | 27JUN23 | STM       | STM32F303K8   | Compiles |
| unity       | 27JUN23 | Microchip | ATtiny        | Fail     | Not enough space on chip + Footnote [1]

## Footnotes

1. `variant` and friends rely on std::is_constructible and similar, which isn't available in AVR environment yet due to lack of c++11 - though some progress is underway.  See https://github.com/malachi-iot/estdlib/issues/7
2.  Unexpected behavior with steady_time::now() see https://github.com/malachi-iot/estdlib/issues/6