#pragma once

#include "traits/allocator_traits.h"
#include "allocators/generic.h"
#include "allocators/fixed.h"

namespace estd {

#ifdef UNUSED
// 100% generic allocator
template <class T>
struct Allocator
{
    typedef T value_type;
    typedef value_type* pointer;
    typedef void* void_pointer;
    typedef const void* const_void_pointer;

    // non-standard, for handle based scenarios
    typedef pointer handle_type;
};
#endif



template <class TPtr> struct pointer_traits;
template <class T> struct pointer_traits<T*>;



}
