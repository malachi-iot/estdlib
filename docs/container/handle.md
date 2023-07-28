# Overview

Document v0.1

# Detail

## estd::handle_descriptor<Allocator>

Describes an allocated peice of memory, including:

- Holds on to an allocator (or a "sparse" allocator if it's global/empty struct)
- The size of the particular allocated chunk
- An integer or similar "handle" which is resolved to a pointer by way of a lock operation

Under the hood, it has four major flags which it specializes on:

- stateful: class or instance level `Allocator``
- has_size: true = allocator tracks size of chunk, false = descriptor manually tracks size
- singular: false = traditional, allocate multiple times.  true = can only allocate one time (aka 'fixed singlebuf' allocator)
- contiguous: always true at this time

Also wraps up familiar allocator operations such as `construct`, `free` etc. but hard wired to the tracked handle and allocator.

## estd::internal::allocator_and_handle_descriptor



## estd::internal::handle_with_offset_raw

More or less a container for a pointer


