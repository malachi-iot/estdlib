#pragma once

#include "../type_traits.h"
#ifdef FEATURE_STD_UTILITY
#include <utility>
#include <memory> // for std::allocator
#else
#include "../utility.h"
#endif
#include <stdint.h> // for uint8_t and friends

namespace estd {

namespace internal {

// experimental feature, has_typedef (lifted from PGGCC-13)
template<typename>
struct has_typedef
{
    typedef void type;
};

}

namespace experimental {

// since < c++11 can't do static constexpr bool functions, we need an alternate
// way to build up specializations based on attribute_traits.  Also has benefit
// of not requiring every allocator to implement true/false on this entire feature
// spread.  Not yet tested or in use as of this writing

template<typename T, typename = void>
struct has_stateful_tag : estd::false_type {};

template<typename T>
struct has_stateful_tag<T, typename estd::internal::has_typedef<typename T::is_stateful_tag_exp>::type> : estd::true_type {};

template<typename T, typename = void>
struct has_singular_tag : estd::false_type {};

template<typename T>
struct has_singular_tag<T, typename estd::internal::has_typedef<typename T::is_singular_tag_exp>::type> : estd::true_type {};

template<typename T, typename = void>
struct has_size_tag : estd::false_type {};

template<typename T>
struct has_size_tag<T, typename estd::internal::has_typedef<typename T::has_size_tag_exp>::type> : estd::true_type {};


}

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
    //typedef std::size_t                         size_type;
    typedef typename TAllocator::size_type      size_type;
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
    static CONSTEXPR bool is_stateful() { return TAllocator::is_stateful(); }

    static CONSTEXPR bool is_stateful_exp = experimental::has_stateful_tag<allocator_type>::value;

    // indicates whether the allocator_type is stateful (requiring an instance variable)
    // or purely static
    static CONSTEXPR bool is_singular() { return TAllocator::is_singular(); }

    static CONSTEXPR bool is_singular_exp = experimental::has_singular_tag<allocator_type>::value;

    // indicates whether handles innately can be queried for their size
    static CONSTEXPR bool has_size() { return TAllocator::has_size(); }

    static CONSTEXPR bool has_size_exp = experimental::has_size_tag<allocator_type>::value;

    static CONSTEXPR value_type invalid_handle() { return TAllocator::invalid_handle(); }

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

    static const value_type& clock(const allocator_type& a, handle_type h, size_type pos = 0)
    {
        return a.clock(h, pos);
    }

    static const value_type& clock(const allocator_type& a, const handle_with_offset& h, size_type pos = 0)
    {
        return a.clock(h, pos);
    }

    static void unlock(allocator_type& a, handle_type h)
    {
        a.unlock(h);
    }

    static void cunlock(const allocator_type& a, handle_type h)
    {
        a.cunlock(h);
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
