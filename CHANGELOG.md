# v0.5.x - XXXXX23

## Quality Updates & Bug Fixes

* Fixing signed/unsigned mismatch in `out_span_streambuf`
* Fixing `estd::optional` bug when using `reset` on enum type
* Adding edge case optimization to `estd::tuple` (see FEATURE_ESTD_SPARSE_TUPLE)

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
