# Overview

Document v0.1

# Scope

# Architecture

## Garbage Collection

Deletion takes two stages:

1. Mark for removal (gc flag)
2. Actual null of entry

Items marked for removal are called "sparse".  In broad terms, an item is in one of 3 states:

* active = non-null key_type. mapped_type content
* null = null key_type. meta content (gc flag = 0)
* sparse = null key_type. meta content (gc flag = 1)

# Structures

## control_type

Alias for `pair<key_type, meta>`

When key is null meta is in gc/metadata mode
When key is not null, meta is in storage mode

This type is designed to be 1:1 castable with `value_type`

## key_type

## mapped_type

## meta

Union of storage and gc-style metadata 

## value_type

Alias for `pair<const key_type, mapped_type>`

# API

## erase_ll

Takes as input `find_result`

Runs destructor, nulls out key and marks GC bit

## erase

## find_ll

Returns a `pair<control_pointer, n>` aka `find_result`

## gc_active

Distantly similar to a `std::realloc`

Moves the given iterator to a null entry elsewhere.  Importantly, the iterator is otherwise the same, active data.  Merely its location has moved to accomodate garbage collection of other entries.

## gc_sparse_ll

Demotes this sparse 'pos' to completely deleted 'null'

# API: Internal Calls (subject to change)

