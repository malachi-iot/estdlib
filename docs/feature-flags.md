# C++ Level

## FEATURE_ESTD_AGGRESSIVE_BITFIELD

Use bit packed structs to sacrifice speed for space

## FEATURE_ESTD_ALGORITHM_OPT

Normally estd aliases out some of the <algorithm> functions in hopes that
your particular platform has a nice optimized version.  This can be disabled
by setting FEATURE_ESTD_ALGORITHM_OPT to 0

## FEATURE_ESTD_CHRONO

If off, estd::chrono largely aliases to std::chrono

> DEBT: Confusing

## FEATURE_ESTD_CHRONO_LITERALS

~~Compilers frequently get mad when we try to make our own flavor of user literals
`s`, `ms`, etc. so this flag enables or disables this feature~~

> Above is maybe what it should be, but isn't right now

Set to true when GCC (not Clang) is used, indicating estd versions of `s`, `ms`
chrono literals are present.

## FEATURE_ESTD_FREERTOS_CHRONO

When true (default when ESTD_OS_FREERTOS is set) maps `estd::freertos_clock` to `estd::steady_clock` 

## FEATURE_ESTD_FREERTOS_THREAD

Enable or disable estd::mutex and similar wrappers for freertos.
Defaults to true.  Note that estd::freertos::mutex and similar are
enabled always.

## FEATURE_ESTD_IS_EMPTY

LLVM and GCC support __is_empty extension, necessary for estd::is_empty
implementation.  This flag indicates the presence of estd::is_empty based
on that criteria

## FEATURE_ESTD_IOS_GCOUNT

Enable or disable 'gcount' feature in istream

## FEATURE_ESTD_NATIVE_CHRONO

If true (defaults to false) disables `FEATURE_STD_CHRONO_CORE` and `FEATURE_STD_CHRONO_CLOCK`

## FEATURE_ESTD_SPARSE_TUPLE

Enables non-standard behavior to reduce size of tuples when empty (0-byte)
size elements are used.  Defaults to true.  'get' will end up returning
a temporary in this case

## FEATURE_ESTD_STREAMBUF_TRAITS

Enables cts/dtr style signaling from streambuf up to istream/ostream.
Streambuf by itself does not exhibit signals outwardly, instead control
structure `signal` is allocated in istream/ostream

# CMake level

## FEATURE_ESTD_INTERFACE

Default: `unset`

When 1, estd compiles as a CMake interface library

## FEATURE_ESTD_FREERTOS

Default: `1`

When `1`, FreeRTOS, if detected as available, is fully enabled.
This means frequent extra includes into our own FreeRTOS headers likely bring in
FreeRTOSConfig.h which may present a headache.  So, that can be forced off via
`0` if FreeRTOS is present but you really don't want to include support.

Specifically this may interest you if you are compiling a lot of different scenarios and have FREERTOS_KERNEL_PATH set to on

A value of `2` indicates ESTD_OS_FREERTOS was specified by cmake as
a compiler def (since it's more descriptive and available than just FREERTOS).  This gets
converted to `1` in `freertos/version.h`

> DEBT: Special value `2` is confusing and is planned to be phased out

## FEATURE_POSIX_CHRONO

## FEATURE_STD_CHRONO_CORE

## FEATURE_STD_CHRONO_CLOCK

Even if FEATURE_STD_CHRONO is present and true, we may only elect to alias out
estd::steady_clock = std::steady_clock if this is true (default 1)