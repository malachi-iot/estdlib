# Overview

Document v0.1

# Detail

Allocated arrays are, in short, array-like containers which depend on an allocator
Allocated arrays are not expected to grow or shrink in size, though a class
like `dynamic_array` can extend and change that

## estd::internal::impl::allocated_array<Allocator>

More or less the core glue to hold together and allocated array.  By way of
`estd::handle_descriptor`, it holds on to:

- allocator itself
- size allocated to the array

Also `estd::handle_descriptor` provides a lot of wrapped functionality for
lock/unlock and things like `construct`, `free` etc all mated to one particular
memory chunk.

## estd::internal::allocated_array<Impl>

Implements familiar core array operations: `at`, `operator[]`, `begin`, etc.

Noteworthy is that `accessor` is employed here rather than direct references, though
`accessor` can potentially be configured to be a reference.  [See documentation on Accessors](concepts.md#accessors)
