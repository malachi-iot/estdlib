# estd

C++ Standard Lib adapted to embedded environments

Influence by, but different than, the great work done here: https://github.com/ETLCPP/etl

## Overview

### Design

Embedded environments have distinctly different memory management needs
than their desktop/server counterparts.  Oore explicit and organized memory
management schemes are desirable.  A general methodology is followed:

| Layer   |   Description   | Dynamic Allocation
| -----   |   ------------- | -
| layer1  | Inline fixed buffer, no size variance or pointers | No
| layer2  | Either fixed buffer+runtime size or buffer pointer+fixed size | No
| layer3  | Buffer pointer and runtime size variable utilized | No

The notion is elimination of virtual functions in favor of impl-type metaprogramming, and naturally no dynamic allocation.

### Compiler Support

We try to enable std-like features sooner than the actual spec, when possible.
For example a fair number of type_traits are available even if you're in c++98 mode

## Feature Highlights

No virtual functions or dynamic allocation for any of these!

### Character Conversion (charconv)

`from_chars` and `to_chars` are available for integer conversions.

### Chrono

Many primitives for basic time handling and comparison are brought in mirroring `std::chrono` namespace.
For bonus fun there's a minimal `steady_clock` implementation for FreeRTOS.

### Locale

Bits and peieces of locale are reinterpreted.  Multiple languages and character sets are supported.  This is
done via templates and not runtime configuration yielding high performance at the cost of broad multiple simultaneous locale support.

Noteworthy is `num_get` whose underlying implementation is adapted to non-blocking scenarios.

### Optional

The `optional` class is present for your convenience.  A few specialized `layer1::optional` also exist to fold memory usage down even further.

### IO Streams

`ostream`/`istream` support is now present.  It is cut down in robustness from the `std` implementation but still very useful.  The upside to reduced features is it's *much* lighter weight than `std`.

Above `ios` mechanism relies on the available and familiar `streambuf` class.

Finally, the implementation has been gently reinterpreted to favor non-blocking behavior.  A deeper discussion on this is in the documentation.

Because the entire `ios` chain is lighter than `std` flavor, it becomes much more viable in all kinds of scenarios.

### Queues

Circular queues are particularly useful in embedded environments, and as such layer1::deque provides exactly that.

A brute force reimplementation of priority_queue is also present, since I needed that for AVRs

### Vectors

Following the layer1-layer3 paradigm, vectors are available for allocation anywhere you want!

### Span

Buffer helpers, and also bringing some extra boost-inspired const_buffer and
mutable_buffer code

### Tuples

Much of the same functionality as you would expect from `std::tuples`.

### Strings

All strings can operate in either null-terimanted or explicit size-tracked modes.  

#### estd::basic_string

Very closely resembles std::string, but utilizes extra behaviors of our allocators.

#### estd::layer1::basic_string

Has the same memory footprint as a traditional char[N] string when null terminated.
When in size-tracked mode, size_type will be the smallest possible unsigned int which
can manage up to N

When N is less than 256, then null-nullterminated and size-tracked
take the same amount of memory since size_type can be uint8_t.

#### estd::layer2::basic_string

Utilizes a pointer to track its underlying buffer and a compile time constant
to designate maximum buffer size.  Additionally, this designation may be 0
in which case the string is unbounded, just like a classic C string.  Has
the same memory footprint as a pointer.

#### estd::layer2::const_string

Since layer2::basic_string maps so well to a classic C string, we have a const_string
to assist in interacting with string literals.  As one may expect, its const nature
limits many of its activities but still a useful construct.

#### estd::layer3::basic_string

Utilizes both a pointer and an instance variable to track buffer and size.
This variable is expected to remain constant after instantiation.

#### estd::layer3::const_string

Similar to layer3::basic_string but based on a const char* for that extra
safety.  Remember, stock std::string doesn't do this because it never points
at memory other than its own malleable set

NOTE: This so far seems to be 1:1 with `string_view`

## Special Thanks

[JetBrains](https://jetbrains.com?from=estdlib) CLion is the tool of choice for this Open Source project.  Their product suite is incredible.  Thank you!

