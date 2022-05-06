# vNEXT - DDMMM22

## Added features

* `estd::char_traits` can now optionally override `std::char_traits`

## Quality Updates

* Vastly reduced stack usage in relationn to `pos_type`
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