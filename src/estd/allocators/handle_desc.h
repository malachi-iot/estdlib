#pragma once

namespace estd { namespace internal {

// is_stateful = is this a stateful (instance) allocator or a global (singleton-style) allocator
// has_size = does this allocator track size of the allocated item
// is_singular = standard allocator behavior is to be able to allocate multiple items.  a singular allocator can only allocate one item
template <class TAllocator, bool is_stateful, bool has_size, bool is_singular>
class handle_descriptor;

}}
