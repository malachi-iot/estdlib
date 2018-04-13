#pragma once

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

template <class TAllocator>
struct allocator_traits
{
    typedef TAllocator                          allocator_type;
    typedef typename TAllocator::value_type     value_type;
    typedef typename TAllocator::pointer        pointer;
    typedef size_t                              size_type;

    // non-standard, for handle based scenarios
    typedef typename TAllocator::handle_type    handle_type;

    typedef typename allocator_type::const_void_pointer     const_void_pointer;

    static handle_type allocate(allocator_type& a, size_type n)
    {
        return a.allocate(n);
    }

    static void deallocate(allocator_type& a, handle_type p, size_type n)
    {
        return a.deallocate(p, n);
    }

    // NOTE: Consider strongly putting nonstandard lock and unlock in here,
    // to accomodate handle-based memory

    static pointer lock(allocator_type& a, handle_type h)
    {
        return h;
    }


    static void unlock(allocator_type& a, handle_type h) {}
};


template <class TPtr> struct pointer_traits;
template <class T> struct pointer_traits<T*>;


}
