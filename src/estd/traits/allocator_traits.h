#pragma once

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

    //typedef typename allocator_type::accessor           accessor;

    // non-standard, and phase this out in favor of 'helpers' to wrap up
    // empty counters
    typedef typename TAllocator::lock_counter           lock_counter;

    static CONSTEXPR handle_type invalid() { return allocator_type::invalid(); }
    static CONSTEXPR bool is_locking() { return allocator_type::is_locking(); }

    // indicates whether the allocator_type is stateful (requiring an instance variable)
    // or purely static
    static CONSTEXPR bool is_stateful() { return true; }

    static handle_type allocate(allocator_type& a, size_type n, const_void_pointer hint = NULLPTR)
    {
        return a.allocate(n);
    }

    static void deallocate(allocator_type& a, handle_type p, size_type n)
    {
        return a.deallocate(p, n);
    }


    static reference lock(allocator_type& a, handle_type h, size_type pos = 0, size_type count = 0)
    {
        return a.lock(h, pos, count);
    }

    static reference lock(allocator_type &a, handle_with_offset h, size_type pos = 0, size_type count = 0)
    {
        return a.lock(h, pos, count);
    }

    static const value_type& clock_experimental(const allocator_type& a, handle_type h, size_type pos = 0)
    {
        return a.clock_experimental(h, pos);
    }

    static const value_type& clock_experimental(const allocator_type& a, const handle_with_offset& h, size_type pos = 0)
    {
        return a.clock_experimental(h, pos);
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

namespace internal {

// lifted from https://stackoverflow.com/questions/9510514/integer-range-based-template-specialisation
template<bool> struct Range;

template<size_t val, typename = Range<true> >
class deduce_fixed_size_t
{};

template<size_t val>
struct deduce_fixed_size_t<val, Range<(0 <= val && val <= 255)> >
{
    typedef uint8_t size_type;
};

template<size_t val>
struct deduce_fixed_size_t<val, Range<(255 < val && val <= 0xFFFF)> >
{
    typedef uint16_t size_type;
};

template<size_t val>
struct deduce_fixed_size_t<val, Range<(0xFFFF < val && val <= 0xFFFFFFFF)> >
{
    typedef uint32_t size_type;
};


}

}
