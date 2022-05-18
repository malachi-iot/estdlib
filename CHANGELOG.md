# vNEXT - DDMMM22

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