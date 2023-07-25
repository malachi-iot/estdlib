#pragma once

namespace estd { namespace internal {

// FIX: eventually use something a bit like our Range<bool> trick in the fixed_size_t finder
template <class TAllocator, bool is_locking>
struct locking_allocator_traits;

}}