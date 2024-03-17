Invoke just basic compliation for Arduino targets to ensure it indeed works

## Results

| Project     |   Date  | Vendor    | Platform      | Result   | Notes |
| ----------- | ------- | --------- | ------------- | -------- | -----
| basic       | 27JUN23 | Microchip | ARM SAMD21 m0 | Partial  | Nearly passes[^2]
| basic       | 09MAR24 | Microchip | AVR 32u4      | Compiles |
| basic       | 25FEB24 | Microchip | AVR 32u4 3.3v | Pass     |
| basic       | 09MAR24 | Microchip | AVR Attiny    | Compiles |
| basic       | 20JUL23 | Microchip | ARM SAM m4    | Pass     |
| cstddef     | 28JUN23 | Microchip | ARM SAMD21 m0 | Pass   | Use 'unity' project instead
| from_chars  | 28JUN23 | Microchip | ARM SAMD21 m0 | Pass   |
| from_chars  | 09MAR24 | Microchip | AVR 32u4      | Compiles |
| from_chars  | 20APR22 | Espressif | ESP32S        | Pass   |
| from_chars  | 09MAR24 | Microchip | ATtiny        | Compiles |
| istream     | 28JUN23 | Microchip | ARM SAMD21 m0 | Pass   | [^3]
| ostream     | 27JUN23 | Microchip | ARM SAMD21 m0 | Pass     |
| ostream     | 16NOV23 | Microchip | AVR 32u4      | Pass     |
| ostream     | 25FEB24 | Microchip | AVR 32u4 3.3v | Pass     |
| ostream     | 27JUN23 | Espressif | ESP32         | Compiles |
| ostream     | 18NOV23 | Espressif | ESP32C3       | Pass     |
| ostream     | 27JUN23 | STM       | STM32F303K8   | Compiles |
| ostream     | 09MAR24 | Microchip | ATtiny        | Compiles |
| string      | 16NOV23 | Microchip | AVR 32u4      | Pass     |
| unity       | 28JUN23 | Microchip | ARM SAMD21 m0 | Pass     |
| unity       | 20JUL23 | Microchip | ARM SAMD51 m4 | Pass     | regular and CAN both
| unity       | 16MAR24 | Microchip | AVR 32u4      | Partial  | Passes except for `variant` subtleties[^1]
| unity       | 09AUG23 | Espressif | ESP32S        | Fail     | FreeRTOS related compile errors
| unity       | 16NOV23 | Espressif | ESP32C3       | Compiles |
| unity       | 17MAR24 | STM       | STM32F303K8   | Compiles |
| unity       | 27JUN23 | Microchip | ATtiny        | Fail     | Not enough space on chip + variant issues[^1]

## Footnotes

[^1]: `variant` and friends rely on std::is_constructible and similar, which is now reimplemented in AVR environment but may have issues - though some progress is underway.  See https://github.com/malachi-iot/estdlib/issues/7
[^2]:  Unexpected behavior with steady_clock::now() see https://github.com/malachi-iot/estdlib/issues/6.  Tests now use arduino_clock, however
[^3]:  Behavior may not be 100% as expected.  See specific README associated with project
