Invoke just basic compliation for Arduino targets to ensure it indeed works

## Results

| Project  |   Date  | Platform            | Result   | Notes |
| -------- | ------- | ------------------- | -------- | -
| basic    | 27JUN23 | Atmel ARM SAMD21 m0 | Compiles | 
| basic    | 27JUN23 | Atmel AVR 32u4      | Fail     | `variant` and friends rely on | -------- |std::is_constructible and similar, which isn't available here
| basic    | 27JUN23 | Atmel ARM SAM m4    | Compiles |
