# estdlib
C++ Standard Lib adapted to embedded environments

Influence by, but different than, the great work done here: https://github.com/ETLCPP/etl

## Design

Since embedded environments often have distinctly different management needs
than their desktop/server counterparts, more explicit and organized memory
management schemes are desirable.  A general methodology is followed:

| Layer   |   Description   | Dynamic Allocation
| -----   |   ------------- | -
| layer1  | Inline fixed buffer, no size variance or pointers | No
| layer2  | Either fixed buffer+runtime size or buffer pointer+fixed size | No
| layer3  | Buffer pointer and runtime size variable utilized | No
| layer4  | Reserved | Undecided
| layer5  | layer3 + virtual function usage | Undecided

layer1-layer3 avoids virtual functions and highly favors template metaprogramming.
Furthermore, layer1-layer3 buffers are of fixed size - no dynamic requesting of
new buffers.

layer4 is reserved but expected to be layer2 + virtual functions or some kind of
indicator of dynamic memory dependency

Many embedded libraries eschew dynamic allocation (ish) behaviors. estdlib takes a slightly different approach.  Static-only allocations *are fully supported and encouraged* but it isn't your only option.  See wiki for more detail

## Compiler Support

We try to enable std-like features sooner than the actual spec, when possible.
For example a fair number of type_traits are available even if you're in c++98 mode

## Platform Support

There is minimal platform abstraction.  It isn't this library's function but
sometimes (like during chrono) it's convenient to do it and have it in estdlib

Also since I use Espressif chips a lot, there's some version-query helpers for
them

# std-like library implementations

## Allocators

Allocators are a superset of C++ std::allocator approach, and include an extra
layer of indirection where an allocation must be locked before a pointer is
available.  This fosters smooth interaction with virtualized (handle-based)
memory, memory pools and other highly specialized allocation scenarios.

## Arrays

### estd::array

Since std::array itself is actually not dynamically allocated, this one is pretty much 1:1 with the standard one.

## Lists

### estd::forward_list

Singly-linked list adapted to handle node allocation in very specific ways.  It can be used in 3 main ways:

1.  "intrusive" where value_type itself contains next-pointers
2.  "semi-intrusive" where value_type is an inline value contained by a forward_list managed node
3.  "unintrusive" where value_type is a reference referred to by a forward_list managed node

It's presently being considered to map 1, 2 and 3 to layer1, layer2 and layer3 forward_lists, respectively

### estd::list

Doubly-linked list having the same capabilities as forward_list with its node allocation.

## Optional

The `optional` class is present for your convenience.  Also a `layer1::optional` is available which has no internal boolean flag and instead depends on a null value of your choosing to represent 'no value' state.

## IO Streams

Rudimentary ostream/istream support is now present.  The API is mostly internal,
but functionally it's shaping up well.  These have been implemented differently
than regular std::ios and friends - it uses templates much more strongly and
does not depend on virtual functions.  A deeper discussion is warranted, expect
better documentation in time.

## Queues

Circular queues are particularly useful in embedded environments, and as such layer1::deque provides exactly that.  A layer2 and layer3 variant are planned.

### estd::priority_queue

In theory std::priority_queue should work out of the box with the estd containers.
However, it mysteriously does not so estd::priority_queue does work, though not
as powerful as std::priority_queue.

## Vectors

Vector is implemented quite similarly to standard library, but utilizes extra features of our allocator approach.  Right now layer1 vectors work well.  Eventually vectors shall be able to operate in at least layer1-layer3 modes

## Maps

Planned

## Span

Buffer helpers, and also bringing some extra boost-inspired const_buffer and
mutable_buffer code

## Chrono

Many primitives for basic time handling and comparison are brought in mirroring `std::chrono` namespace and for bonus fun there's a minimal `steady_clock` implementation for FreeRTOS

## Tuples

Much of the same functionality as you would expect from `std::tuples`.

## Strings

All strings can operate in either null-terimanted or explicit size-tracked modes.  

### estd::basic_string

Very closely resembles std::string, but utilizes extra behaviors of our allocators.

### estd::layer1::basic_string

Has the same memory footprint as a traditional char[N] string when null terminated.
When in size-tracked mode, size_type will be the smallest possible unsigned int which
can manage up to N

When N is less than 256, then null-nullterminated and size-tracked
take the same amount of memory since size_type can be uint8_t.

### estd::layer2::basic_string

Utilizes a pointer to track its underlying buffer and a compile time constant
to designate maximum buffer size.  Additionally, this designation may be 0
in which case the string is unbounded, just like a classic C string.  Has
the same memory footprint as a pointer.

### estd::layer2::const_string

Since layer2::basic_string maps so well to a classic C string, we have a const_string
to assist in interacting with string literals.  As one may expect, its const nature
limits many of its activities but still a useful construct.

### estd::layer3::basic_string

Utilizes both a pointer and an instance variable to track buffer and size.
This variable is expected to remain constant after instantiation.

### estd::layer3::const_string

Similar to layer3::basic_string but based on a const char* for that extra
safety.  Remember, stock std::string doesn't do this because it never points
at memory other than its own malleable set

## shared_ptr

This is a modified implementation of regular `shared_ptr` to suit our non-dynamic-allocation scenarios.

# Experimental code

Subject/Observer code was experimented on here but now lives in `embr` library.  Do not use the implementation here

There is a memory_pool implementation, it's quite functional but its API is not quite stable.