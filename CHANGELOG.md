# v0.X.X - XXXXX25

## Added Features

* `layer1::unordered_map` now available
* https://github.com/malachi-iot/estdlib/issues/76 added `estd::accumulate`
* Adding crude estd::hash for layer1 and layer2 strings

## Quality Updates & Bug Fixes

* `unordered_map` work has also upgraded `unordered_set`
* https://github.com/malachi-iot/estdlib/issues/84, https://github.com/malachi-iot/estdlib/issues/92 upgrading Catch2 to 3.8.0
* https://github.com/malachi-iot/estdlib/issues/95 Behind the scenes cleanup of arrays, strings

## Known Issues & Limitations

* `unordered_map`, `unordered_set` has only core functionality.
  Missing are node_type and hints.  Also, they've undergone only limited testing

# v0.8.3 - 23FEB25

## Added Features

* https://github.com/malachi-iot/estdlib/issues/83 `ESTD_FLAGS` constexpr enum flags helper
* https://github.com/malachi-iot/estdlib/issues/78 `estd::pair` now has ==, != and < operators

## Quality Updates & Bug Fixes

* https://github.com/malachi-iot/estdlib/issues/77 byteswap code much cleaner and compiles on AVR now
* https://github.com/malachi-iot/estdlib/issues/80 & https://github.com/malachi-iot/estdlib/issues/54 improved ESP-IDF chrono support

# v0.8.2 - 10DEC24

## Added Features

* https://github.com/malachi-iot/estdlib/issues/70 CMake compiler option macro helper
* https://github.com/malachi-iot/estdlib/issues/69 `get<T>(tuple)` now available
* https://github.com/malachi-iot/estdlib/issues/58 ESP-IDF logging emulation

## Quality Updates & Bug Fixes

* https://github.com/malachi-iot/estdlib/issues/68 `freertos::wrapper::task::create_static` now truly calls `xTaskCreateStatic`
* https://github.com/malachi-iot/estdlib/issues/66 `tuple.visit()` now works with references too

# v0.8.1 - 31OCT24

## Added Features

* `estd::num_put` for bools & integers now available
    * Supports foreign locales, encodings, 16 bit characters and more
    * Compile-time tuning options
* `estd::variadic::values` gets `where` and `contains()`
* https://github.com/malachi-iot/estdlib/issues/49 rudimentary byteswap
* https://github.com/malachi-iot/estdlib/issues/51 enhanced operators for `units`
* https://github.com/malachi-iot/estdlib/issues/53 `function_traits` now active

## Known Issues & Limitations

* A full `num_put` is extensive functionality.  Currently unsupported are floats, comma and decimal placement

## Quality Updates & Bug Fixes

* https://github.com/malachi-iot/estdlib/issues/48 more flexible span constructor
* `basic_ispanbuf` and friends promoted from `experimental` to `estd`

# v0.8.0 - 08JUN24

## Added Features

* Adding simplistic beta-quality memory pool.  Located in `estd::internal::memory::pool`
* Adding `make_heap`, `push_heap` and `pop_heap`

## Quality Updates & Bug Fixes

* https://github.com/malachi-iot/estdlib/issues/40 Fixed glitch in Streambuf c++20 concept
* https://github.com/malachi-iot/estdlib/issues/39 Upgraded & cleaned out internal `detail::function`
* AVR pgm support additional testing & refinement
* Dogfooding internal `units` code into `chrono` support
* Fixed glitch in `variadic::values` when using non-integer types

## Known Issues & Limitations

* Our heapify functions are crude and won't be as optimized as `std`.
  Still, it's better than nothing

# v0.7.2 - 18MAR24

## Quality Updates & Bug Fixes

* https://github.com/malachi-iot/estdlib/issues/35 Top-level CMakeLists.txt now usable
* Massive cleanup of `estd/limits.h`:
    * https://github.com/malachi-iot/estdlib/issues/36 fixed type_traits dependency
    * https://github.com/malachi-iot/estdlib/issues/37 Deducing `long long` far more effectively now

# v0.7.1 - 16MAR24

## Added Features

* [#31](https://github.com/malachi-iot/estdlib/issues/31) Added `estd::detail::iostream`
* [#34](https://github.com/malachi-iot/estdlib/issues/34) Added `uint_least64_t` and some other `numeric_traits` related goodies

## Quality Updates & Bug Fixes

* Improved clang compatibility, especially with c++20 concepts and bipbuffer
* Fixed bug: 64-bit numeric_limits sometimes resolved to 32-bit
* [#24](https://github.com/malachi-iot/estdlib/issues/24) Switched default size_type of strings from `int16` -> `size_t` (can still override with policy)
* [#32](https://github.com/malachi-iot/estdlib/issues/32) Fixed various tuple::visit edge cases
* [#33](https://github.com/malachi-iot/estdlib/issues/33) Fixed minor glitch with `dec`, `hex` manipulator dependencies

# v0.7.0 - 02MAR24

## Added Features

* `estd::tuple` now can operate in sparse mode, meaning it can fold down to 0 size if all its members are also 0 size.
* Wrapping willemt's awesome bipbuffer as `layer1::bipbuf` and `layer3::bipbuf`
    * See https://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist

## Added Features (internal)

Early access - marked internal mainly due to naming and API flux, but passes tests:

* Added `out_buffered_stringbuf` and `out_bipbuf_streambuf` wrapper for "true" streambuf operations.  These sit in front of a raw streambuf for additional caching.
* Core units capability - including `bytes` and `percent` and SI helpers for converting to string

## Quality Updates & Bug Fixes

* Bringing back `c_str()` to layer1 and layer2 strings
* Various AVR and `estd::variadic` cleanup
* Fixed bug in `to_chars` (https://github.com/malachi-iot/estdlib/issues/28)

# v0.6.2 - 26OCT23

## Added Features

* Formally an esp-idf component now
* Adding floating point support to `num_get`
* Formalizing `estd::detail::to_chars`, an even lower-level interpretation of `to_chars`

## Quality Updates & Bug Fixes

* Using CMake's `FetchContent` now to acquire unit test frameworks rather than submodules
* Minor breaking change: selector disambiguation
    * v1 namespace - "selector" might mean API to select or result of selecting
    * v2 namespace - "selector" only means API to select
* Fixed bug where hex 'a' was encoded incorrectly during `to_chars`

## Known Issues & Limitations

* `from_chars` doesn't yet get floating point, even though `num_get` does

# v0.6.1 - 16AUG23

## Added Features

* `estd::detail::string` is a more manageable way to pass strings around
* Experimental/partial support for AVR PGM strings: `estd::v0::avr::pgm_string`

## Quality Updates & Bug Fixes

* `estd::tuple` now has conversion constructor
* `estd::variant` on AVR now usable, with caveats (see next session)
* String support code got a major scrub, though still a lot of crust in there
* `estd::vector` no longer does naughty `memmove` with non-trivial types

## Known Issues & Limitations

* `estd::is_constructible` is broken on AVR.  Possibly this is due to `-fpermissive` default flag
* `estd::variant` on AVR caveats:
    * Using `is_convertible` rather than `is_constructible`, for assignment
      operator, so your mileage may vary when using more exotic types and constructors
    * `is_constructible` being how it is may negatively affect other areas.  So far unit tests are OK though.
    * move semantic seems glitchy, destructs don't seem to get called

# v0.6.0 - 20JUL23

*NOTICE*: License changed from **MIT** to **APACHE 2.0**

## Added Features

* Added `estd::variant`
* Added `estd::variadic` namespace
    * `variadic::types` for compile-time management of `class ...Types`
    * `variadic::values` for compile-time management of `T ...Values` (overlaps with `estd::index_sequence`)
    * `variadic::selector` for compile-time evaluation of `class ...Types`
    * `variadic::visitor` for runtime visitor pattern applied to `class ...Types`
* Added ESTD_VERSION macro for compile-time version query

## Quality Updates & Bug Fixes

* `is_base_of` no longer caught up on access modifierrs
* General Arduino, AVR, c++03 compatibility
* `estd::optional` and `estd::expected` much more robust
    * shares code base with `estd::variant`
    * when available, exceptions are now supported
* `estd::num_get` and by extension `istream` integer parsing now returns min/max value on overflow
* `estd::priority_queue` works on more targets

## Known Issues & Limitations

* `estd::variant` not functional on AVR, probably bugs in https://github.com/malachi-iot/estdlib/issues/7

# v0.5.1 - 18MAY23

## Added Features

* Adding `setw` and `setfill` to ostream
* Adding `estd::expected` and `estd::unexpected`

## Quality Updates & Bug Fixes

* Fixing signed/unsigned mismatch in `out_span_streambuf`
* Fixing `estd::optional` bug when using `reset` on enum type
* Adding edge case optimization to `estd::tuple` (see FEATURE_ESTD_SPARSE_TUPLE)

## Known Issues & Limitations

* `estd::expected`:
    * Not well tested outside of c++11 GCC, though designed to be compatible with c++03
    * Various edge cases like returning && value() not yet implemented

# v0.5.0 - 16JAN23

## Quality Updates & Bug Fixes

* int -> string conversion general quality upgrades, better guard against non-int
  * Fixed bug in `estd::to_string` inhibiting it from resolving certain integers
* `estd::optional` gets a revamp, some bugfixes, and a nifty bool specialization
* breaking change: `basic_istream` and `basic_ostream` are now moved from `internal` to `detail` namespace
* AVR MPU compatibility

# v0.4.2 - 03JAN23

## Added Features

* Added new `estd::internal::undefined_array`.  This operates nearly identical to `estd::array` except that default constructor for elements is not invoked
* Added non-standard `length` to `numeric_limits` which finds maximum string length for int conversions

## Quality Updates & Bug Fixes

* `estd::layer1::vector` no longer requires a default contructor in its managed types
* `estd::span` rework and quality improvement
* RISC-V int -> string conversion repaired, and general platform int -> string conversion improved
* Fixed `char_traits` specialization glitch
* Misc c++03 compatibility

## Known Issues & Limitations

* Undecided what a good name for `undefined_array` really should be, thus its internal status.  Considering strongly `estd::uarray`
* Breaking changes: `layer2::array`, `layer3::array` are now deprecated and moved to `estd::legacy` namespace.  `estd::span` is a much better fit

# v0.4.1 - 14DEC22

## Added Features

* FreeRTOS:
    * `estd::freertos::timer`
    * `estd::freertos::event_groups`

## Quality Updates & Bug Fixes

* CMake: INTERFACE mode fixed, general cleanup, experimental `install` capability
* `estd::freertos::queue`
    * aligning isr and non-isr send signatures (breaking change)
    * adding convenience max-duration/timeout methods
* `estd::basic_istream` now reflects numeric parsing status in good() and has associated bool operator

## Known Issues & Limitations

* FreeRTOS:
    * `estd::freertos::timer` static mode lacks proper unit test
* CMake: `install` capability not well tested

# v0.4.0 - 01DEC22

## Added Features

* Raspberry Pi Pico now tested and supported
    * Includes lightweight `estd::basic_pico_ostream`
    * Includes experimental `chrono` support
* Added official FreeRTOS support: 
    * `estd::freertos::mutex`
    * `estd::freertos::counting_semaphore`
    * `estd::freertos::queue`
    * above has static and dynamic allocation variants
    * aliases of above to `estd::mutex` and `estd::counting_semaphore`
* Added low level variant of `std::function`: `estd::detail::function`


## Quality Updates & Bug Fixes

* estd::span now has default constructor
* CMake project name now `estd` rather than `estd_lib`
    * The latter still exists as an alias for legacy compatibility
    * Added also `malachi-iot::estd` alias in case of collisions
* estd::priority_queue now can use instanced `Compare`

## Known Issues & Limitations

* FreeRTOS support:
    * `estd::freertos::recursive_mutex` not well tested or implemented
    * `estd::freertos::mutex` binary mode is confusing and might get removed.  Use `binary_semaphore`

# v0.3.3 - 16AUG22

## Added Features

* chrono:
    * Added `hh_mm_ss`
* ratio:
    * `ratio` now auto reduces its `type`, as called for by std spec
    * `ratio_add` now present
    * `ratio_divide` and `ratio_multiply` auto reduce as briefl
       mentioned by std spec
* c++03 flavor of `tuple` now available
* type_traits: added `underlying_type` and `type_identity`

## Quality Updates & Bug Fixes

* `ratio_divide` and `ratio_multiply` are now aliased out for
  compilers that support the feature.  The non aliased specialized
  structs are moved now to `detail` namespace, which is a
  breaking change for c++03 targets.
* `tuple` and `pair` cleanup and minor improvements
* chrono: general cleanup
* `platformio` unity tests working again
* Minor c++20 compatibility

# v0.3.2 - 17JUN22

## Added Features

* Adding `remove_cv_t` and friends

## Quality Updates & Bug Fixes

* `chrono_literals` operators: 
    * disabling on CLang as it generates warnings and doesn't compile right
    * was returning `seconds` incorrectly instead of `milliseconds`, etc. 
* `vector`:
    * Fixed deep `layer1::vector` glitch which sometimes broke `emplace`
    * Fixed bug which disallowed resizing up to maximum layer1 size

# v0.3.1 - 27MAY22

## Added Features

* An iterated bool version of num_get is now available, called `bool_get`
* Adding more bits and pieces to chrono: + operator, != operator and literals

## Quality Updates & Bug Fixes

* Fixed last-character bug in `internal::chooser`
* Fixed `ostream` integer output glitch on m0

## Known Issues & Limitations

* `internal::chooser` very much a work in progress and only reliable for the limited use
  cases it's currently employed into

# v0.3.0 - 18MAY22

## Added Features

* `internal::basic_istream` can be configured in blocking or non blocking mode via `TPolicy`
* `internal::flagged_istream` now available to make `TPolicy` interaction easier
* istream and ostream wrappers for Arduino's `Stream` and `Print` classes
* Groundwork for compile-time locale support, including `num_get`, `moneypunct` and `ctype`
    * num_get is supported by a class named `iterated::num_get` which is, as
      the name suggests, a non blocking state machine
* Extraction operator >> now supports int types
* `istreambuf_iterator` has elevated from experimental to available API

## Quality Updates & Bug Fixes

* C++0x compatibility, especially where streambuf/iostreams are involved
* unit test under Windows (CLion) fixed
* More granular POSIX support for scenarios which only have partial POSIX capability (MINGW)

## Known Issues & Limitations

* istream:
    * Blocking support is new and is likely to have deficiencies
    * Arduino wrapper *only* works in blocking mode
* POSIX: Too much feature flagging depends on the POSIX flags, erroneous and confusing
* locale:
    * support exists at this time to support `num_get` int processing specifically.  It is very much not a complete locale implementation.
    * changing the locale on an istream is difficult at the moment

## Additional Notes

Although istreams can now be configured as blocking or nonblocking, underlying streambufs continue
to be nonblocking.  At some point we'll probably need a blocking streambuf paradigm as well, but so
far we've avoided it.

Custom locales actually work, but slightly fiddly at the moment.

# v0.2.1 - 06MAY22

## Added Features

* `estd::char_traits` can now optionally override `std::char_traits`

## Quality Updates

* Vastly reduced stack usage in relation to `pos_type`
* Finagled platformio package generation into behaving again
* Maintenance to test code overall

## Known Issues

* platformio compilation of tests doesn't treat `estdlib` as a formal lib, instead referring to it as `src`

# v0.2.0 - 20APR22

Many updates since 0.1.7:

* streambufs now cleaned up and useful
* further target compatibility: VisualDSP/C++98, esp-idf, platformio
* estd::priority_queue added
* rounding out to_chars, adding in over-base-10 compatibility
* adding in rudimentary estd::num_get implementation
* better STD lib detection in the first place

Known issues:

* C++98 targets don't compile with some of the new queue code. I'm not sure anyone cares at this point
* estd::priority_queue does depend on std::push_heap (no dynamic allocation, but std lib required)
* Big stack usage on esp32 streambuf relating to `pos_type`
