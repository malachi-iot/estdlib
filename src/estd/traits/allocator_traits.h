#pragma once

#include "../internal/fwd/memory.h"

#include "../type_traits.h"
#include "../utility.h"
#include "../internal/utility.h" // for has_member_base & ESTD_FN_HAS_METHOD
#include "../memory.h" // for std::allocator
#include "../cstdint.h"
#include "../internal/deduce_fixed_size.h"
#include "../internal/container/handle_with_offset.h"
#include "../allocators/tags.h"

#include "../internal/container/traditional_accessor.h"
#include "../internal/container/iterator.h"

#include "fwd.h"

namespace estd {

namespace internal {

#ifdef UNUSED_CODE
// lifted directly from https://jguegant.github.io/blogs/tech/sfinae-introduction.html
template <class T> struct hasSerialize : has_member_base
{
    // Two overloads for yes: one for the signature of a normal method, one is for the signature of a const method.
    // We accept a pointer to our helper struct, in order to avoid to instantiate a real instance of this type.
    // std::string (C::*)() is function pointer declaration.
    template <typename C> static yes& test(reallyHas<std::string (C::*)(), &C::serialize>* /*unused*/) { }
    template <typename C> static yes& test(reallyHas<std::string (C::*)() const, &C::serialize>* /*unused*/) { }

    // The famous C++ sink-hole.
    // Note that sink-hole must be templated too as we are testing test<T>(0).
    // If the method serialize isn't available, we will end up in this method.
    template <typename> static no& test(...) { /* dark matter */ }

    // The constant used as a return value for the test.
    // The test is actually done here, thanks to the sizeof compile-time evaluation.
    static CONSTEXPR bool value = sizeof(test<T>(0)) == sizeof(yes);
};
#endif


ESTD_FN_HAS_METHOD(void, destroy,)


template <class T> struct has_construct_method : has_member_base
{
    template <typename C> static CONSTEXPR yes& test(reallyHas<void (C::*)(), &C::construct>* /*unused*/)
    { return yes_value; } // returning yes_value to quell warnings on older compilers

    // The famous C++ sink-hole.
    // Note that sink-hole must be templated too as we are testing test<T>(0).
    // If the method serialize isn't available, we will end up in this method.
    template <typename> static CONSTEXPR no& test(...) { return no_value; }

    // The constant used as a return value for the test.
    // The test is actually done here, thanks to the sizeof compile-time evaluation.
    static CONSTEXPR bool value = sizeof(test<T>(0)) == sizeof(yes);
};


#ifdef FEATURE_CPP_VARIADIC
template <class TAlloc2, class T, class... TArgs>
static typename estd::enable_if<internal::has_construct_method<TAlloc2>::value, int>::type
    construct_sfinae(TAlloc2& a, T* p, TArgs&&... args)
{
    a.construct(p, std::forward<TArgs>(args)...);
    return true;
}

// no construct method in underlying allocator, so use our own placement new
template <class TAlloc2, class T, class... TArgs>
static typename estd::enable_if<!internal::has_construct_method<TAlloc2>::value, int>::type
    construct_sfinae(TAlloc2& a, T* p, TArgs&&... args)
{
    new (static_cast<void*>(p)) T(std::forward<TArgs>(args)...);
    return false;
}
#endif

template <class TAlloc, class T>
static typename estd::enable_if<internal::has_destroy_method<TAlloc>::value, bool>::type
    destroy_sfinae(TAlloc& a, T* p)
{
    a.destroy(p);
    return true;
}

template <class TAlloc, class T>
static typename estd::enable_if<!internal::has_destroy_method<TAlloc>::value, bool>::type
    destroy_sfinae(TAlloc& a, T* p)
{
    p->~T();
    return false;
}

/*
// experimental feature, has_typedef (lifted from PGGCC-13)
template<typename>
struct has_typedef
{
    typedef void type;
}; */


struct allocator_locking_preference
{
    enum _
    {
        /// Allocator does not real locking at all.  Any lock API is using a pointer shim
        none,
        /// Allocator does traditional explicit lock/unlock calls at allocator_traits level
        standard,
        /// Allocator prefers iterator-level locking, which has its own set of moves
        iterator
    };
};

/// interact with actual underlying locking allocator
template <class TAllocator>
struct locking_allocator_traits<TAllocator, true>
{
    typedef TAllocator                          allocator_type;
    typedef typename TAllocator::value_type     value_type;
    typedef value_type*                         pointer;
    typedef const value_type*                   const_pointer;
    typedef typename TAllocator::size_type      size_type;
    typedef typename TAllocator::handle_type    handle_type;

    // EXPERIMENTAL
    // Some kind of global memory pool Allocator type might be empty/stateless
    typedef typename conditional<is_empty<allocator_type>::value,
        allocator_type,
        allocator_type&>::type allocator_valref;

    // handle_with_size is phased out with advent of has_size_tag coupled with
    // outside-of-allocator specializations
    //typedef typename TAllocator::handle_with_size       handle_with_size;
    typedef typename TAllocator::handle_with_offset     handle_with_offset;

    typedef value_type&                         reference; // deprecated in C++17 but relevant for us due to lock/unlock

    ESTD_CPP_CONSTEXPR(14) static reference lock(allocator_valref a, handle_type h, size_type pos = 0, size_type count = 0)
    {
        return a.lock(h, pos, count);
    }

    ESTD_CPP_CONSTEXPR(14) static reference lock(allocator_valref a, handle_with_offset h, size_type pos = 0, size_type count = 0)
    {
        return a.lock(h, pos, count);
    }

    constexpr static const value_type& clock(const allocator_type& a, handle_type h, size_type pos = 0, size_type count = 0)
    {
        return a.clock(h, pos, count);
    }

    constexpr static const value_type& clock(const allocator_type& a, const handle_with_offset& h, size_type pos = 0)
    {
        return a.clock(h, pos);
    }

    ESTD_CPP_CONSTEXPR(14) static void unlock(allocator_type& a, handle_type h)
    {
        a.unlock(h);
    }

    ESTD_CPP_CONSTEXPR(14) static void cunlock(const allocator_type& a, handle_type h)
    {
        a.cunlock(h);
    }

    static CONSTEXPR handle_type invalid() { return allocator_type::invalid(); }

    // Non standard, for scenarios in which a consumer specifically wants to treat
    // a handle region as a container of T.  For locking types, they MUST provide
    // an iterator type
    typedef typename allocator_type::iterator iterator;

    static CONSTEXPR allocator_locking_preference::_ locking_preference =
        allocator_locking_preference::standard;
};


/// shim for non locking activities
template <class Allocator>
struct locking_allocator_traits<Allocator, false>
{
    using allocator_type = Allocator;
    using value_type = typename allocator_type::value_type;

    typedef value_type*                         pointer;
    typedef const value_type*                   const_pointer;
    typedef typename allocator_type::size_type      size_type;
    typedef pointer                             handle_type;

    // EXPERIMENTAL
    // Probably always nonlocking allocators are empty...
    typedef typename conditional<is_empty<allocator_type>::value,
        //const allocator_type&,
        allocator_type,
        allocator_type&>::type allocator_valref;

    // handle_with_size is phased out with advent of has_size_tag coupled with
    // outside-of-allocator specializations
    //typedef typename TAllocator::handle_with_size       handle_with_size;

    typedef estd::internal::handle_with_offset_raw<pointer>    handle_with_offset;

    typedef value_type&                         reference; // deprecated in C++17 but relevant for us due to lock/unlock

    static reference lock(const allocator_type&, handle_type h, size_type pos = 0, size_type count = 0)
    {
        return *(h + pos);
    }

    static reference lock(const allocator_type&, handle_with_offset h, size_type pos = 0, size_type count = 0)
    {
        return *(h.handle() + pos);
        //return a.lock(h, pos, count);
    }

    static const value_type& clock(const allocator_type& a, handle_type h, size_type pos = 0, size_type count = 0)
    {
        return *(h + pos);
    }

    static reference clock(const allocator_type &a, handle_with_offset h, size_type pos = 0, size_type count = 0)
    {
        return *(h.handle() + pos);
    }

    static void unlock(const allocator_type& a, handle_type h)
    {
    }

    static void cunlock(const allocator_type& a, handle_type h)
    {
    }

    static CONSTEXPR handle_type invalid() { return NULLPTR; }

    // Non standard, for scenarios in which a consumer specifically wants to treat
    // a handle region as a container of T
    using iterator = typename internal::locking_iterator<
        allocator_type,
        traditional_accessor<value_type>,
        internal::locking_iterator_modes::shim>;

    static CONSTEXPR allocator_locking_preference::_ locking_preference =
        allocator_locking_preference::none;
};


}

// NOTE: May very well be better off using inbuilt version and perhaps extending it with
// our own lock mechanism
// NOTE: I erroneously made our burgeoning custom allocators not-value_type aware
template <class TAllocator>
struct allocator_traits
#ifdef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
        :
        internal::locking_allocator_traits<TAllocator, internal::has_locking_tag<TAllocator>::value >
#endif
{
    typedef TAllocator                          allocator_type;
    typedef typename TAllocator::value_type     value_type;
    typedef typename TAllocator::pointer        pointer;
    typedef typename TAllocator::size_type      size_type;

    typedef typename internal::get_difference_type<allocator_type>::type difference_type;
    // FIX: Do a SFINAE extraction of difference type
    // doesn't work, still tries to resolve allocator_type::difference_type always
    /*
    typedef estd::conditional<
        experimental::has_difference_type<allocator_type>::value,
            typename allocator_type::difference_type,
            estd::make_signed<size_type> >
        difference_type; */

    typedef value_type&                         reference; // deprecated in C++17 but relevant for us due to lock/unlock

    // non-standard, for handle based scenarios
#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    typedef typename TAllocator::handle_type            handle_type;
    //typedef typename TAllocator::handle_with_size       handle_with_size;
    typedef typename TAllocator::handle_with_offset     handle_with_offset;
#else
    typedef internal::locking_allocator_traits<TAllocator, internal::has_locking_tag<TAllocator>::value > base_t;
    typedef typename base_t::handle_type handle_type;
    typedef typename base_t::handle_with_offset handle_with_offset;
#endif

    typedef typename allocator_type::const_void_pointer     const_void_pointer;

    //typedef typename allocator_type::accessor           accessor;

#ifdef FEATURE_ESTD_ALLOCATOR_LOCKCOUNTER
    // non-standard, and phase this out in favor of 'helpers' to wrap up
    // empty counters
    typedef typename TAllocator::lock_counter           lock_counter;
#endif

#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static CONSTEXPR handle_type invalid() { return allocator_type::invalid(); }

    static CONSTEXPR bool is_locking() { return allocator_type::is_locking(); }

    // indicates whether the allocator_type is stateful (requiring an instance variable)
    // or purely static
    static CONSTEXPR bool is_stateful() { return TAllocator::is_stateful(); }

    // indicates whether the allocator_type is stateful (requiring an instance variable)
    // or purely static
    static CONSTEXPR bool is_singular() { return TAllocator::is_singular(); }

    // indicates whether handles innately can be queried for their size
    static CONSTEXPR bool has_size() { return TAllocator::has_size(); }
#endif

    static CONSTEXPR bool is_locking_exp = internal::has_locking_tag<allocator_type>::value;

    static CONSTEXPR bool is_stateful_exp = internal::has_stateful_tag<allocator_type>::value;

    static CONSTEXPR bool is_singular_exp = internal::has_singular_tag<allocator_type>::value;

    static CONSTEXPR bool has_size_exp = internal::has_size_tag<allocator_type>::value;

    // NOTE: contiguous is an experimental and incomplete feature, and one can safely assume all
    // allocators are marked as TRUE for is_contiguous as this time
    static CONSTEXPR bool is_contiguous_exp = !internal::has_noncontiguous_tag<allocator_type>::value;

    //static CONSTEXPR value_type invalid_handle() { return TAllocator::invalid_handle(); }

    static handle_type allocate(allocator_type& a, size_type n, const_void_pointer hint = NULLPTR)
    {
        return a.allocate(n);
    }

    static void deallocate(allocator_type& a, handle_type p, size_type n)
    {
        return a.deallocate(p, n);
    }


#ifndef FEATURE_ESTD_STRICT_DYNAMIC_ARRAY
    static reference lock(allocator_type& a, handle_type h, size_type pos = 0, size_type count = 0)
    {
        return a.lock(h, pos, count);
    }

    static reference lock(allocator_type &a, handle_with_offset h, size_type pos = 0, size_type count = 0)
    {
        return a.lock(h, pos, count);
    }

    static const value_type& clock(const allocator_type& a, handle_type h, size_type pos = 0, size_type count = 0)
    {
        return a.clock(h, pos, count);
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
#endif

    static size_type max_size(const allocator_type& a)
    {
        // note that a.max_size is no longer required (though spec
        // strongly implies it's optionally permitted) in C++17, though
        // allocator_traits::max_size is not
        return a.max_size();
    }

#ifdef FEATURE_CPP_VARIADIC
private:
public:
    // TODO: Consider changing T* p to handle_type for locking variant
    //       note that we'd have to do a handle_with_offset version too since
    //       this call is often used in an allocated-array capacity
    template <class T, class... TArgs>
    static void construct(allocator_type& a, T* p, TArgs&&... args)
    {
        internal::construct_sfinae(a, p, std::forward<TArgs>(args)...);
        //new (static_cast<void*>(p)) T(std::forward<TArgs>(args)...);
    }
#endif

    // just for feature parity with construct
    // same concepts apply, namely:
    // a) SFINAE approach should be coded here
    // b) consider a handle_type/handle_with_offset variety
    template <class T>
    static void destroy(allocator_type& a, T* p)
    {
        internal::destroy_sfinae(a, p);
    }
};


}
