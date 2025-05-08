# platformio unit tests

Shares unity tests with esp32 rtos + visualdsp

## Interesting Emulation Options

### AVR

https://docs.platformio.org/en/latest/plus/debug-tools/simavr.html
https://docs.platformio.org/en/stable/advanced/unit-testing/simulators/simavr.html

`pio test -e simavr --without-uploading`

#### 08MAY25

Works on Rover (debian), errors out:

```
/home/malachi/.platformio/packages/tool-simavr/bin/simavr: error while loading shared libraries: libsimavr.so.1: cannot open shared object file: No such file or directory

```

on c131-debian
