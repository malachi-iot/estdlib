#pragma once

#include "../../platform.h"
#include <utility> // for std::forward and std::size_t
#include <memory> // for std::allocator
#include "../type_traits.h"

namespace estd {

template <class TAllocator>
struct allocator_traits;




// NOTE: May very well be better off using inbuilt version and perhaps extending it with
// our own lock mechanism
// NOTE: I erroneously made our burgeouning custom allocators not-value_type aware
template <class TAllocator>
struct allocator_traits
{
    typedef TAllocator                          allocator_type;
    typedef typename TAllocator::value_type     value_type;
    typedef typename TAllocator::pointer        pointer;
    typedef std::size_t                         size_type;
    typedef value_type&                         reference; // deprecated in C++17 but relevant for us due to lock/unlock

    // non-standard, for handle based scenarios
    typedef typename TAllocator::handle_type            handle_type;
    typedef typename TAllocator::handle_with_size       handle_with_size;
    typedef typename TAllocator::handle_with_offset     handle_with_offset;
    typedef typename allocator_type::const_void_pointer     const_void_pointer;

    // non-standard, and phase this out in favor of 'helpers' to wrap up
    // empty counters
    typedef typename TAllocator::lock_counter           lock_counter;

    static handle_type allocate(allocator_type& a, size_type n, const_void_pointer hint = NULLPTR)
    {
        return a.allocate(n);
    }

    static void deallocate(allocator_type& a, handle_type p, size_type n)
    {
        return a.deallocate(p, n);
    }


    static reference lock(allocator_type& a, handle_type h)
    {
        return a.lock(h);
    }

    static void unlock(allocator_type& a, handle_type h)
    {
        a.unlock(h);
    }

    static size_type max_size(const allocator_type& a)
    {
        // note that a.max_size is no longer required (though spec
        // strongly implies it's optionally permitted) in C++17, though
        // allocator_traits::max_size is not
        return a.max_size();
    }

#ifdef FEATURE_CPP_VARIADIC
    template <class T, class... TArgs>
    static void construct(allocator_type& a, T* p, TArgs&&... args)
    {
        new (static_cast<void*>(p)) T(std::forward<TArgs>(args)...);
    }
#endif
};


// semi-kludgey, a way to shoehorn in existing std::allocator using our extended
// locking mechanism.  Eventually use type_traits + SFINAE to auto deduce non-
// existing handle_type, etc.
template<class T>
struct allocator_traits<::std::allocator<T>> :
        public ::std::allocator_traits<::std::allocator<T>>
{
    typedef ::std::allocator_traits<::std::allocator<T>> base_t;

    typedef typename base_t::pointer handle_type;
};

}