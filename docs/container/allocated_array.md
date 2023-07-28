# Overview

Document v0.1

# Detail

Allocated arrays are, in short, array-like containers which depend on an allocator
Allocated arrays are not expected to grow or shrink in size, though a class
like `dynamic_array` can extend and change that

## estd::internal::impl::allocated_array<Allocator>

- Fed an allocator
- 

## estd::internal::allocated_array

## estd::handle_descriptor

- Holds on to an allocator (or a "sparse" allocator if it's global/empty struct)