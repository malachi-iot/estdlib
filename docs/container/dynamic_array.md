# Overview

Document v0.1

# estd::internal::impl::dynamic_array<Allocator, Policy>

Layers resizeability over `estd::internal::impl::allocated_array`
Null termination (or not) awareness by way of semi-specialization on policy

# estd::internal::dynamic_array

Generally represents a resizeable array.

Accounts for locking/unlocking behavior and therefore need not be contiguous, but in reality today it always is.

