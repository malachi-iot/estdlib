## chrono

## timer

## unity

## Results

These tests are all for variants of Espressif ESP32

|   Date  | Project  | Board                | Chip           | esp-idf  | Result | Notes |
| ------- | -------- | -------------------- | -------------- | -------  | ------ | ----- |
| 05DEC22 | chrono   | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v5.0     | Pass   |
| 08JUN23 | chrono   | ESP32 Lolin Generic  | ESP32          | v5.0.2   | Pass   |
| 31JUL23 | chrono   | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v5.1     | Pass   |
| 07DEC22 | ios      | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v4.4.3   | Pass   |
| 02JAN23 | ios      | ESP32-C3-DevKitM-1   | ESP32C3        | v5.0     | Pass   | 
| 20JUL23 | ios      | ESP32 Lolin Generic  | ESP32          | v5.0.3   | Pass   |
| 19DEC23 | ios      | WaveShare C6-DevKit  | ESP32C6        | v5.1.2   | Pass   |
| 07DEC22 | timer    | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v4.4.3   | Pass   |
| 15JUN23 | timer    | ESP32-C3-DevKitM-1   | ESP32C3        | v5.0.2   | Pass   |
| 07DEC22 | unity    | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v4.4.3   | Pass   |
| 03JAN23 | unity    | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v5.0     | Pass   |
| 15JUN23 | unity    | ESP32-C3-DevKitM-1   | ESP32C3        | v5.0.2   | Pass   |
| 29SEP23 | unity    | ESP32-C3-DevKitM-1   | ESP32C3        | v5.1.1   | Pass   |
| 19DEC23 | unity    | WaveShare C6-DevKit  | ESP32C6        | v5.1.2   | Pass   |
| 18NOV23 | unity    | ESP32C3 Xiao         | ESP32C3        | v5.1.2   | Pass   |
| 20JUL23 | unity    | ESP32 Lolin Generic  | ESP32          | v5.0.3   | Pass   |
| 29SEP23 | unity    | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v5.1.1   | Pass   |
| 29SEP23 | unity    | Lilygo QT Pro        | ESP32S3        | v5.1.1   | Pass*  | Intermittent failures[^1]
| 04FEB24 | unity    | Seeed Xiao           | ESP32S3        | v5.1.2   | Pass   |

[^1]: Fails on `test_std_system_clock`.  Likely 18NOV23 commit repairs this

