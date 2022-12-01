# C++ Level

## FEATURE_ESTD_AGGRESIVE_BITFIELD

Use bit packed structs to sacrifice speed for space

## FEATURE_ESTD_ALGORITHM_OPT

Normally estd aliases out some of the <algorithm> functions in hopes that
your particular platform has a nice optimized version.  This can be disabled
by setting FEATURE_ESTD_ALGORITHM_OPT to 0

## FEATURE_ESTD_FREERTOS_THREAD

Enable or disable estd::mutex and similar wrappers for freertos.
Defaults to true.  Note that estd::freertos::mutex and similar are
enabled always.

## FEATURE_ESTD_IOS_GCOUNT

Enable or disable 'gcount' feature in istream

# CMake level

## FEATURE_ESTD_INTERFACE

Default: `unset`

When 1, estd compiles as a CMake interface library

## FEATURE_ESTD_FREERTOS

Default: `1`

Normally FreeRTOS, if detected as available, defaults to fully enabled.
This means frequent extra includes into FreeRTOS headers including possibly
FreeRTOSConfig.h which may present a headache.  So, that can be forced off
If FreeRTOS is present but you really don't want to include support.

Specifically this may interest you if you are compiling a lot of different scenarios and have FREERTOS_KERNEL_PATH set to on
