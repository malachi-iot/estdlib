## std-like library implementations

Unless otherwise noted, these features are backported to c++98.

### Allocators

Allocators are a superset of C++ std::allocator approach, and include an extra
layer of indirection where an allocation must be locked before a pointer is
available.  This fosters smooth interaction with virtualized (handle-based)
memory, memory pools and other highly specialized allocation scenarios.

### Arrays

#### estd::array

Since std::array itself is actually not dynamically allocated, this one is pretty much 1:1 with the standard one.

### Lists

#### estd::forward_list

Singly-linked list adapted to handle node allocation in very specific ways.  It can be used in 3 main ways:

1.  "intrusive" where value_type itself contains next-pointers
2.  "semi-intrusive" where value_type is an inline value contained by a forward_list managed node
3.  "unintrusive" where value_type is a reference referred to by a forward_list managed node

It's presently being considered to map 1, 2 and 3 to layer1, layer2 and layer3 forward_lists, respectively

#### estd::list

Doubly-linked list having the same capabilities as forward_list with its node allocation.

### Queues

#### estd::priority_queue

A brute force reimplementation of priority_queue.

### Maps

Planned

### shared_ptr

This is a modified implementation of regular `shared_ptr` to suit our non-dynamic-allocation scenarios.

