# vNEXT - DDMMMYY

## Added Features

* `internal::basic_istream` can be configured in blocking or non blocking mode via `TPolicy`
* `internal::flagged_istream` now available to make `TPolicy` interaction easier
* istream and ostream wrappers for Arduino's `Stream` and `Print` classes

## Quality Updates

* C++0x compatibility, especially where streambuf/iostreams are involved

## Known Issues

* istream:
    * Blocking support is new and is likely to have deficiencies
    * Arduino wrapper *only* works in blocking mode
    * extraction operator only supports strings

## Additional Notes

Although istreams can now be configured as blocking or nonblocking, underlying streambufs continue
to be nonblocking.  At some point we'll probably need a blocking streambuf paradigm as well, but so
far we've avoided it.

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