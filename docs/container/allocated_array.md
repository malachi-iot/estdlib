# Overview

Document v0.1

# Detail

Allocated arrays are, in short, array-like containers which depend on an allocator
Allocated arrays are not expected to grow or shrink in size, though a class
like `dynamic_array` can extend and change that

## estd::internal::impl::allocated_array<Allocator>

More or less a shim around [estd::handle_descriptor](handle.md#estdhandle_descriptor)

Only exists to typedef `policy_type` for usage down the line by dynamic_array

## estd::internal::allocated_array<Impl>

Implements familiar core array operations: `at`, `operator[]`, `begin`, etc.

Requires from Impl:

- offset
- get_accessor

Noteworthy is that `accessor` is employed here rather than direct references, though
`accessor` can potentially be configured to be a reference.  [See documentation on Accessors](concepts.md#accessors)
