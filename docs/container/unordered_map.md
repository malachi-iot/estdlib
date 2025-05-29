# Overview

Document v0.1

# Scope

This document descibes behavior of `unordered_map`

# Architecture

A feature of `unordered_map` architecture is despite `erase` or `insert` operations,
pointers to existing items *remain stable* by default.  Outlined here is how that operates,
though those details are largely opaque to the programmer.

## Key Nullability

Items are indicated as null or active by way of its key.  This can cause problems if your key needs to be "0".  In that scenario you have two options:

* Activate `Nullability` capability
* Leverage `estd::optional` for your keys

## Garbage Collection

Deletion takes two stages:

1. Mark for removal (gc flag)
2. Actual null of entry

Items marked for removal are called "sparse".  In broad terms, an item is in one of 3 states:

* active = non-null key_type. mapped_type content
* null = null key_type. meta content (gc flag = 0)
* sparse = null key_type. meta content (gc flag = 1)

An "empty" item is null or sparse

### null item

Null entries are fair game for allocation of an active item
AND designate no further searching for items in this bucket is needed.

### sparse item

Sparse entries are a placeholder to 'extend' a bucket forward.  This way null entries
don't interrupt discovery of active entries.

This way a bucket can effectively have a bunch of empty slots from prior
deletions without needing to move the active entry to the front of the bucket.

# Structures

## control_type

Alias for `pair<key_type, meta>`

When key is null meta is in gc/metadata mode
When key is not null, meta is in storage mode

This type is designed to be 1:1 castable with `value_type`

## find_result

Alias for `pair<control_pointer, size_type>`

Where `size_type` id particular `index()`, or npos if
not found.

## key_type

## mapped_type

## meta

Union of storage and gc-style metadata 

## value_type

Alias for `pair<const key_type, mapped_type>`

# API

Described here are API deviating from `std::unordered_map`

## erase_ll

Takes as input `find_result`

Runs destructor, nulls out key and marks GC bit

## erase

## find_ll

Returns a `pair<control_pointer, n>` aka `find_result`

## gc_active

"Garbage Collect Active Item"

Distantly similar to a `std::realloc`

Moves the given iterator to a null entry elsewhere.  Importantly, the iterator is otherwise the same, active data.  Merely its location has moved to accomodate garbage collection of other entries.

## gc_sparse_ll

"Garbage Collect Sparse Item (low level)"

Demotes this sparse 'pos' to completely deleted 'null'

# API: Internal Calls (subject to change)

