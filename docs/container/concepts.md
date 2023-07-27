# Overview

Document v0.1

NOT referring to c++20 concepts feature

# Allocators

`estd` builds on regular std allocators concept and adds a few notions:

- locking/unlocking
- iterator awareness

# Locking

Locking effectively translates abstract non contiguous memory into concrete
physical memory.  Possible examples of this could be:

- memory mapped file: generally file is NOT in memory, but when locked, it is.
- fragmented memory: allocated memory can be moved around when not locked (in this case, they MAY be in fact contiguous)

# Iterators

Allocator-provided iterators strongly resemble traditional ones.

A notable difference is iterators are one a major interface points for
locking.  That said, locking is still very much not ready for use so
locking APIs are a (necessary) formality which largely just shim around
regular pointer access

# Accessors

Per iteration in an iterator, one has a go-between to get at physical
memory called an accessor.

